#pragma once
#include <QWidget>
#include <QPixmap>
#include <QTimer>
#include <opencv2/core.hpp>
#include "CameraScanner.h"

class QLabel;
class QPushButton;
class QFrame;

// ScanPage (Member 1 - Desktop GUI)
// Provides UI to scan barcodes from uploaded images (Member 3 service)
// or real-time webcam feed (Member 4 service).
class ScanPage : public QWidget {
    Q_OBJECT

public:
    explicit ScanPage(QWidget* parent = nullptr);
    ~ScanPage() override;

    // Safely releases webcam hardware if active
    void stopCameraIfRunning();

private slots:
    void onUploadImageClicked();
    void onStartCameraClicked();
    void onStopCameraClicked();
    void onScanAgainClicked();
    void onCameraTimerTick();
    void onCopyResultClicked();

private:
    void setupUi();
    void showStatus(const QString& message, const QString& type);
    void clearStatus();
    void setResult(const QString& code, bool success, const QString& format = "");
    void clearResult();
    static QImage matToQImage(const cv::Mat& mat);

    QPushButton* m_uploadBtn         = nullptr;
    QPushButton* m_startCameraBtn    = nullptr;
    QPushButton* m_stopCameraBtn     = nullptr;
    QPushButton* m_scanAgainBtn      = nullptr;
    QPushButton* m_copyBtn           = nullptr;

    QLabel*      m_previewLabel      = nullptr;
    QLabel*      m_statusLabel       = nullptr;
    QFrame*      m_resultCard        = nullptr;
    QLabel*      m_resultCodeLabel   = nullptr;
    QLabel*      m_resultFormatLabel = nullptr;
    QLabel*      m_sourceInfoLabel   = nullptr;

    QTimer*      m_cameraTimer       = nullptr;
    CameraScanner m_cameraScanner;

    QString      m_lastScannedCode;
    bool         m_isCameraMode      = false;
};
