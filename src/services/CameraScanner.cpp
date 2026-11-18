#include "CameraScanner.h"
#include "CodeValidator.h"
#include "BarcodeDecoder.h"

#include <opencv2/imgproc.hpp>

CameraScanner::~CameraScanner() { stop(); }

OperationResult CameraScanner::start(int cameraIndex)
{
    OperationResult r;
    stop();

    // DirectShow first, then let OpenCV pick any backend.
    if (!capture_.open(cameraIndex, cv::CAP_DSHOW))
        capture_.open(cameraIndex, cv::CAP_ANY);

    if (!capture_.isOpened()) {
        r.message = "Camera unavailable or busy. Close other apps using the camera and try again.";
        return r;
    }

    paused_ = false;
    frameCounter_ = 0;
    emptyFrameCount_ = 0;
    r.success = true;
    r.message = "Camera started.";
    return r;
}

void CameraScanner::stop()
{
    if (capture_.isOpened())
        capture_.release();
    paused_ = false;
}

bool CameraScanner::isRunning() const { return capture_.isOpened(); }

void CameraScanner::resumeScanning() { paused_ = false; }

void CameraScanner::setDecodeInterval(int everyNthFrame)
{
    decodeInterval_ = everyNthFrame < 1 ? 1 : everyNthFrame;
}

OperationResult CameraScanner::scanNextFrame(cv::Mat& frame)
{
    OperationResult r;

    if (!capture_.isOpened()) {
        r.message = "Camera is not running.";
        return r;
    }

    cv::Mat grabbed;
    if (!capture_.read(grabbed) || grabbed.empty()) {
        if (++emptyFrameCount_ >= 30) {
            stop();
            r.message = "Camera stopped delivering frames (disconnected?).";
        } else {
            r.message = "Waiting for camera frame...";
        }
        return r;
    }
    emptyFrameCount_ = 0;
    frame = grabbed;

    if (paused_) {
        r.message = "Scan complete. Press scan again to continue.";
        return r;
    }

    if (++frameCounter_ % decodeInterval_ != 0) {
        r.message = "Scanning...";
        return r;
    }

    r = decodeFrame(grabbed);
    if (r.success)
        paused_ = true;   // pause after a successful scan
    return r;
}

OperationResult CameraScanner::decodeFrame(const cv::Mat& frame)
{
    // Delegate directly to the centralized barcode decoder service
    OperationResult r = decodeBarcodeMat(frame);
    
    // Adjust message wording slightly for camera context if needed
    if (r.success) {
        r.message = "Scanned successfully.";
    }
    return r;
}