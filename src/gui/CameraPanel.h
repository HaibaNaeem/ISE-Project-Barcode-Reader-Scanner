#pragma once
// OWNER: Member 5. Qt widget that shows the camera and reports scanned barcodes.
// Uses CameraScanner (no Qt, no threads) from a QTimer, so the window never freezes.
#include <QElapsedTimer>
#include <QLabel>
#include <QString>
#include <QTimer>
#include <QWidget>
#include "CameraScanner.h"

class CameraPanel : public QWidget {
    Q_OBJECT
public:
    explicit CameraPanel(QWidget* parent = nullptr);
    ~CameraPanel() override;

    void start();                    // opens the camera; if missing, shows a message and the app keeps running
    void stop();                     // releases the camera
    bool isRunning() const { return scanner_.isRunning(); }

signals:
    void barcodeScanned(QString sixDigitCode);
    void cameraMessage(QString message);   // e.g. "Camera unavailable..." (MainWindow can show it in the banner)

private slots:
    void onTick();                   // called every ~30 ms
    void onResume();                 // called ~0.8 s after a scan

private:
    void showMessage(const QString& text);
    void showFrame(const cv::Mat& bgrFrame);

    CameraScanner scanner_;
    QLabel* preview_;
    QLabel* status_;
    QTimer tickTimer_;               // 30 ms
    QTimer resumeTimer_;             // single shot, 800 ms
    QString lastCode_;
    QElapsedTimer sinceLastCode_;    // same code is ignored for 2 s
    bool haveLastCode_ = false;
};
