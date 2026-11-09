#include "ScanPage.h"
#include "Theme.h"
#include "ImageDecoder.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QFileDialog>
#include <QClipboard>
#include <QGuiApplication>
#include <QFileInfo>
#include <opencv2/imgproc.hpp>

ScanPage::ScanPage(QWidget* parent)
    : QWidget(parent)
{
    setupUi();

    m_cameraTimer = new QTimer(this);
    m_cameraTimer->setInterval(30); // ~33 FPS for smooth live video
    connect(m_cameraTimer, &QTimer::timeout, this, &ScanPage::onCameraTimerTick);
}

ScanPage::~ScanPage()
{
    stopCameraIfRunning();
}

void ScanPage::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(18);

    // Header section
    auto* headerLayout = new QVBoxLayout();
    headerLayout->setSpacing(4);

    auto* heading = new QLabel("Scan Barcode", this);
    heading->setProperty("class", "pageHeading");

    auto* subtitle = new QLabel("Read barcodes from uploaded image files or live webcam stream.", this);
    subtitle->setProperty("class", "pageSubtitle");

    headerLayout->addWidget(heading);
    headerLayout->addWidget(subtitle);
    mainLayout->addLayout(headerLayout);

    // Controls Card Frame
    auto* controlsCard = new QFrame(this);
    controlsCard->setProperty("class", "card");
    auto* controlsCardLayout = new QVBoxLayout(controlsCard);
    controlsCardLayout->setContentsMargins(20, 16, 20, 16);
    controlsCardLayout->setSpacing(12);

    auto* controlsLabel = new QLabel("Select Input Source:", controlsCard);
    controlsLabel->setProperty("class", "sectionTitle");
    controlsCardLayout->addWidget(controlsLabel);

    auto* buttonsRow = new QHBoxLayout();
    buttonsRow->setSpacing(12);

    m_uploadBtn = new QPushButton("Upload Image File", controlsCard);
    m_uploadBtn->setProperty("class", "primaryBtn");
    m_uploadBtn->setMinimumHeight(42);
    m_uploadBtn->setCursor(Qt::PointingHandCursor);

    m_startCameraBtn = new QPushButton("Start Camera", controlsCard);
    m_startCameraBtn->setProperty("class", "successBtn");
    m_startCameraBtn->setMinimumHeight(42);
    m_startCameraBtn->setCursor(Qt::PointingHandCursor);

    m_stopCameraBtn = new QPushButton("Stop Camera", controlsCard);
    m_stopCameraBtn->setProperty("class", "dangerBtn");
    m_stopCameraBtn->setMinimumHeight(42);
    m_stopCameraBtn->setEnabled(false);
    m_stopCameraBtn->setCursor(Qt::PointingHandCursor);

    m_scanAgainBtn = new QPushButton("Scan Again", controlsCard);
    m_scanAgainBtn->setProperty("class", "secondaryBtn");
    m_scanAgainBtn->setMinimumHeight(42);
    m_scanAgainBtn->setVisible(false);
    m_scanAgainBtn->setCursor(Qt::PointingHandCursor);

    buttonsRow->addWidget(m_uploadBtn);
    buttonsRow->addWidget(m_startCameraBtn);
    buttonsRow->addWidget(m_stopCameraBtn);
    buttonsRow->addWidget(m_scanAgainBtn);
    controlsCardLayout->addLayout(buttonsRow);

    mainLayout->addWidget(controlsCard);

    // Status Banner (hidden by default)
    m_statusLabel = new QLabel(this);
    m_statusLabel->setObjectName("statusBanner");
    m_statusLabel->setTextFormat(Qt::RichText);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setVisible(false);
    mainLayout->addWidget(m_statusLabel);

    // Main workspace: Left preview, Right result card
    auto* workspaceLayout = new QHBoxLayout();
    workspaceLayout->setSpacing(18);

    // Preview Card Frame (Left)
    auto* previewCard = new QFrame(this);
    previewCard->setProperty("class", "card");
    auto* previewLayout = new QVBoxLayout(previewCard);
    previewLayout->setContentsMargins(18, 16, 18, 16);
    previewLayout->setSpacing(10);

    auto* previewHeading = new QLabel("Live Preview / Selected Image", previewCard);
    previewHeading->setProperty("class", "sectionTitle");
    previewLayout->addWidget(previewHeading);

    m_previewLabel = new QLabel(previewCard);
    m_previewLabel->setObjectName("previewBox");
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setMinimumSize(440, 260);
    m_previewLabel->setText("No camera or image active.\nClick 'Upload Image File' or 'Start Camera'.");
    previewLayout->addWidget(m_previewLabel, 1);

    m_sourceInfoLabel = new QLabel(previewCard);
    m_sourceInfoLabel->setStyleSheet("color: #64748B; font-size: 12px;");
    m_sourceInfoLabel->setAlignment(Qt::AlignCenter);
    m_sourceInfoLabel->setVisible(false);
    previewLayout->addWidget(m_sourceInfoLabel);

    workspaceLayout->addWidget(previewCard, 3);

    // Result Card Frame (Right)
    m_resultCard = new QFrame(this);
    m_resultCard->setProperty("class", "card");
    m_resultCard->setMinimumWidth(260);
    auto* resultLayout = new QVBoxLayout(m_resultCard);
    resultLayout->setContentsMargins(20, 20, 20, 20);
    resultLayout->setSpacing(14);

    auto* resultHeading = new QLabel("Decoded Result", m_resultCard);
    resultHeading->setProperty("class", "sectionTitle");
    resultLayout->addWidget(resultHeading);

    auto* resultBox = new QFrame(m_resultCard);
    resultBox->setStyleSheet("background-color: #F8FAFC; border: 1.5px solid #CBD5E1; border-radius: 10px; padding: 12px;");
    auto* resultBoxLayout = new QVBoxLayout(resultBox);
    resultBoxLayout->setContentsMargins(12, 12, 12, 12);
    resultBoxLayout->setAlignment(Qt::AlignCenter);

    auto* codeBadge = new QLabel("6-DIGIT CODE", resultBox);
    codeBadge->setStyleSheet("color: #64748B; font-size: 11px; font-weight: 700; letter-spacing: 1px;");
    codeBadge->setAlignment(Qt::AlignCenter);
    resultBoxLayout->addWidget(codeBadge);

    m_resultCodeLabel = new QLabel("------", resultBox);
    m_resultCodeLabel->setObjectName("resultCodeLabel");
    m_resultCodeLabel->setAlignment(Qt::AlignCenter);
    resultBoxLayout->addWidget(m_resultCodeLabel);

    m_resultFormatLabel = new QLabel("Symbology: None", resultBox);
    m_resultFormatLabel->setStyleSheet("color: #64748B; font-size: 12px;");
    m_resultFormatLabel->setAlignment(Qt::AlignCenter);
    resultBoxLayout->addWidget(m_resultFormatLabel);

    resultLayout->addWidget(resultBox);

    m_copyBtn = new QPushButton("Copy Code to Clipboard", m_resultCard);
    m_copyBtn->setProperty("class", "secondaryBtn");
    m_copyBtn->setMinimumHeight(38);
    m_copyBtn->setEnabled(false);
    m_copyBtn->setCursor(Qt::PointingHandCursor);
    resultLayout->addWidget(m_copyBtn);

    resultLayout->addStretch();
    workspaceLayout->addWidget(m_resultCard, 2);

    mainLayout->addLayout(workspaceLayout, 1);

    // Event Connections
    connect(m_uploadBtn, &QPushButton::clicked, this, &ScanPage::onUploadImageClicked);
    connect(m_startCameraBtn, &QPushButton::clicked, this, &ScanPage::onStartCameraClicked);
    connect(m_stopCameraBtn, &QPushButton::clicked, this, &ScanPage::onStopCameraClicked);
    connect(m_scanAgainBtn, &QPushButton::clicked, this, &ScanPage::onScanAgainClicked);
    connect(m_copyBtn, &QPushButton::clicked, this, &ScanPage::onCopyResultClicked);
}

void ScanPage::stopCameraIfRunning()
{
    if (m_cameraTimer && m_cameraTimer->isActive()) {
        m_cameraTimer->stop();
    }
    if (m_cameraScanner.isRunning()) {
        m_cameraScanner.stop();
    }
    m_isCameraMode = false;
    m_startCameraBtn->setEnabled(true);
    m_stopCameraBtn->setEnabled(false);
}

void ScanPage::onUploadImageClicked()
{
    // Stop live camera first if running
    stopCameraIfRunning();
    m_scanAgainBtn->setVisible(false);

    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Select Barcode Image File",
        "",
        "Image Files (*.png *.jpg *.jpeg *.bmp);;All Files (*)"
    );

    if (filePath.isEmpty()) {
        return; // User canceled dialog
    }

    // Load and preview selected image
    QPixmap pixmap(filePath);
    if (pixmap.isNull()) {
        m_previewLabel->clear();
        m_previewLabel->setText("Failed to load selected image file.");
        m_sourceInfoLabel->setVisible(false);
        clearResult();
        showStatus("Invalid image file or format not supported.", "error");
        return;
    }

    // Display scaled image in preview
    QPixmap scaled = pixmap.scaled(
        m_previewLabel->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );
    m_previewLabel->setPixmap(scaled);

    m_sourceInfoLabel->setText(
        QString("File: %1  |  Dimensions: %2 x %3 px")
        .arg(QFileInfo(filePath).fileName())
        .arg(pixmap.width())
        .arg(pixmap.height())
    );
    m_sourceInfoLabel->setVisible(true);

    // Call Member 3's Image Decoder Service
    OperationResult result = ImageDecoder::decodeFile(filePath.toStdString());

    if (result.success) {
        setResult(QString::fromStdString(result.value), true, "Code 128 / Code 39");
        showStatus(QString::fromStdString(result.message) + QString(" (Code: %1)").arg(QString::fromStdString(result.value)), "success");
    } else {
        clearResult();
        showStatus(QString::fromStdString(result.message), "error");
    }
}

void ScanPage::onStartCameraClicked()
{
    clearStatus();
    clearResult();

    // Start camera via Member 4's CameraScanner service
    OperationResult startResult = m_cameraScanner.start(0);
    if (!startResult.success) {
        showStatus(QString::fromStdString(startResult.message), "error");
        m_previewLabel->setText("Camera unavailable.\nCheck connection and ensure no other application is using it.");
        m_sourceInfoLabel->setVisible(false);
        return;
    }

    m_isCameraMode = true;
    m_lastScannedCode.clear();

    m_startCameraBtn->setEnabled(false);
    m_stopCameraBtn->setEnabled(true);
    m_scanAgainBtn->setVisible(false);

    m_sourceInfoLabel->setText("Camera stream: Active (DirectShow/Any)");
    m_sourceInfoLabel->setVisible(true);

    showStatus("Camera started. Hold a barcode clearly in front of the lens.", "info");

    // Start UI update timer
    m_cameraTimer->start();
}

void ScanPage::onStopCameraClicked()
{
    stopCameraIfRunning();

    m_previewLabel->clear();
    m_previewLabel->setText("Camera stopped.\nClick 'Start Camera' to resume or 'Upload Image File'.");
    m_sourceInfoLabel->setText("Camera: Stopped");

    showStatus("Camera scanning stopped.", "info");
}

void ScanPage::onScanAgainClicked()
{
    m_cameraScanner.resumeScanning();
    m_scanAgainBtn->setVisible(false);
    clearResult();
    showStatus("Scanning resumed. Point camera at barcode.", "info");
}

void ScanPage::onCameraTimerTick()
{
    if (!m_cameraScanner.isRunning()) {
        m_cameraTimer->stop();
        m_startCameraBtn->setEnabled(true);
        m_stopCameraBtn->setEnabled(false);
        showStatus("Camera stopped delivering frames.", "error");
        return;
    }

    cv::Mat frame;
    OperationResult scanResult = m_cameraScanner.scanNextFrame(frame);

    // Display captured frame if valid
    if (!frame.empty()) {
        QImage qimg = matToQImage(frame);
        if (!qimg.isNull()) {
            QPixmap pix = QPixmap::fromImage(qimg);
            m_previewLabel->setPixmap(pix.scaled(
                m_previewLabel->size(),
                Qt::KeepAspectRatio,
                Qt::FastTransformation
            ));
        }
    }

    // Check if barcode was detected
    if (scanResult.success) {
        QString detectedCode = QString::fromStdString(scanResult.value);
        m_lastScannedCode = detectedCode;

        setResult(detectedCode, true, "Detected Barcode");
        showStatus(QString("Barcode scanned successfully: %1").arg(detectedCode), "success");

        // CameraScanner automatically pauses decoding on success.
        // Show "Scan Again" button so user can scan another barcode.
        m_scanAgainBtn->setVisible(true);
    }
}

void ScanPage::onCopyResultClicked()
{
    QString code = m_resultCodeLabel->text().trimmed();
    if (code.isEmpty() || code == "------") {
        return;
    }

    QClipboard* clipboard = QGuiApplication::clipboard();
    clipboard->setText(code);
    showStatus(QString("Code %1 copied to clipboard!").arg(code), "success");
}

void ScanPage::setResult(const QString& code, bool success, const QString& format)
{
    if (success) {
        m_resultCodeLabel->setText(code);
        m_resultCodeLabel->setStyleSheet("font-size: 32px; font-weight: 800; color: #059669; letter-spacing: 4px;");
        m_resultFormatLabel->setText(QString("Symbology: %1").arg(format.isEmpty() ? "Code 128 / Code 39" : format));
        m_copyBtn->setEnabled(true);
    } else {
        clearResult();
    }
}

void ScanPage::clearResult()
{
    m_resultCodeLabel->setText("------");
    m_resultCodeLabel->setStyleSheet("font-size: 32px; font-weight: 800; color: #0F172A; letter-spacing: 4px;");
    m_resultFormatLabel->setText("Symbology: None");
    m_copyBtn->setEnabled(false);
}

void ScanPage::showStatus(const QString& message, const QString& type)
{
    m_statusLabel->setText(Theme::formatStatusHtml(message, type));
    m_statusLabel->setVisible(true);
}

void ScanPage::clearStatus()
{
    m_statusLabel->clear();
    m_statusLabel->setVisible(false);
}

QImage ScanPage::matToQImage(const cv::Mat& mat)
{
    if (mat.empty()) {
        return QImage();
    }

    switch (mat.type()) {
    case CV_8UC3: {
        // OpenCV stores BGR, convert to RGB for Qt
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        return QImage(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step), QImage::Format_RGB888).copy();
    }
    case CV_8UC1: {
        return QImage(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_Grayscale8).copy();
    }
    case CV_8UC4: {
        cv::Mat rgba;
        cv::cvtColor(mat, rgba, cv::COLOR_BGRA2RGBA);
        return QImage(rgba.data, rgba.cols, rgba.rows, static_cast<int>(rgba.step), QImage::Format_RGBA8888).copy();
    }
    default:
        return QImage();
    }
}
