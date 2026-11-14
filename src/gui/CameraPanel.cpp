#include "CameraPanel.h"
#include <QImage>
#include <QPixmap>
#include <QVBoxLayout>
#include <opencv2/imgproc.hpp>

namespace {
constexpr int kTickMs = 30;           // how often we grab a frame
constexpr int kResumeMs = 800;        // pause after a scan before scanning again
constexpr qint64 kSameCodeMs = 2000;  // ignore the same code again for this long
}

CameraPanel::CameraPanel(QWidget* parent) : QWidget(parent) {
    preview_ = new QLabel(tr("Camera is off"), this);
    preview_->setAlignment(Qt::AlignCenter);
    preview_->setMinimumSize(320, 240);
    preview_->setStyleSheet("background:#222; color:#ddd;");
    status_ = new QLabel(this);
    status_->setWordWrap(true);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(preview_, 1);
    layout->addWidget(status_);

    tickTimer_.setInterval(kTickMs);
    resumeTimer_.setSingleShot(true);
    resumeTimer_.setInterval(kResumeMs);
    connect(&tickTimer_, &QTimer::timeout, this, &CameraPanel::onTick);
    connect(&resumeTimer_, &QTimer::timeout, this, &CameraPanel::onResume);
}

CameraPanel::~CameraPanel() { stop(); }

void CameraPanel::start() {
    if (scanner_.isRunning()) return;
    OperationResult r = scanner_.start(0);
    if (!r.success) {
        preview_->setPixmap(QPixmap());
        preview_->setText(tr("Camera not available"));
        showMessage(QString::fromStdString(r.message));      // the app keeps running
        return;
    }
    haveLastCode_ = false;
    showMessage(tr("Camera started. Hold a barcode in front of the camera."));
    tickTimer_.start();
}

void CameraPanel::stop() {
    tickTimer_.stop();
    resumeTimer_.stop();
    scanner_.stop();
    preview_->setPixmap(QPixmap());
    preview_->setText(tr("Camera is off"));
}

void CameraPanel::onTick() {
    cv::Mat frame;
    OperationResult r = scanner_.scanNextFrame(frame);
    if (!frame.empty()) showFrame(frame);

    if (!scanner_.isRunning()) {                 // scanner stopped itself: camera unplugged
        tickTimer_.stop();
        preview_->setPixmap(QPixmap());
        preview_->setText(tr("Camera not available"));
        showMessage(QString::fromStdString(r.message));
        return;
    }
    if (!r.success) return;                      // "Scanning...", "Waiting for frame...", paused

    // Success: the scanner has paused itself. Give the same item time to leave the view.
    const QString code = QString::fromStdString(r.value);
    const bool repeat = haveLastCode_ && code == lastCode_ && sinceLastCode_.elapsed() < kSameCodeMs;
    if (!repeat) {
        lastCode_ = code;
        haveLastCode_ = true;
        sinceLastCode_.restart();
        showMessage(tr("Scanned %1").arg(code));
        emit barcodeScanned(code);
    }
    resumeTimer_.start();                        // resume scanning in ~0.8 s either way
}

void CameraPanel::onResume() {
    if (scanner_.isRunning()) scanner_.resumeScanning();
}

void CameraPanel::showMessage(const QString& text) {
    status_->setText(text);
    emit cameraMessage(text);
}

void CameraPanel::showFrame(const cv::Mat& bgr) {
    cv::Mat rgb;
    if (bgr.channels() == 3)      cv::cvtColor(bgr, rgb, cv::COLOR_BGR2RGB);
    else if (bgr.channels() == 1) cv::cvtColor(bgr, rgb, cv::COLOR_GRAY2RGB);
    else return;
    QImage image(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step), QImage::Format_RGB888);
    preview_->setPixmap(QPixmap::fromImage(image).scaled(preview_->size(), Qt::KeepAspectRatio, Qt::FastTransformation));
}
