#pragma once
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include "CommonTypes.h"

// Camera scanning service (Member 4).
// No Qt and no threads: the GUI calls scanNextFrame() from a QTimer
// (e.g. every 30 ms), so the window never freezes.
class CameraScanner {
public:
    CameraScanner() = default;
    ~CameraScanner();
    CameraScanner(const CameraScanner&) = delete;
    CameraScanner& operator=(const CameraScanner&) = delete;

    // Opens the camera. success=false + message if unavailable or busy.
    OperationResult start(int cameraIndex = 0);

    // Releases the camera. Safe to call more than once.
    void stop();

    bool isRunning() const;

    // Grabs one frame and (every Nth frame) tries to decode it.
    //  - 'frame' receives the captured image so the GUI can display it.
    //  - success=true  -> value holds a valid six-digit code; scanning pauses.
    //  - success=false -> message says why (scanning, no barcode, wrong content...).
    OperationResult scanNextFrame(cv::Mat& frame);

    // Call after a successful scan to scan again ("scan again" button).
    void resumeScanning();

    // Decode only every Nth frame to save CPU (default 3).
    void setDecodeInterval(int everyNthFrame);

    // Same decode logic can be shared with Member 3's uploaded-image decoder.
    static OperationResult decodeFrame(const cv::Mat& frame);

private:
    cv::VideoCapture capture_;
    bool paused_ = false;
    int frameCounter_ = 0;
    int decodeInterval_ = 3;
    int emptyFrameCount_ = 0;
};