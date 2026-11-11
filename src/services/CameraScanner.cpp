#include "CameraScanner.h"
#include "CodeValidator.h"

#include <opencv2/imgproc.hpp>
#include <ZXing/ReadBarcode.h>

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
        paused_ = true;   // pause after a successful scan (plan section 4.4)
    return r;
}

OperationResult CameraScanner::decodeFrame(const cv::Mat& frame)
{
    OperationResult r;

    if (frame.empty()) {
        r.message = "Empty image.";
        return r;
    }

    // ZXing reads a grayscale buffer.
    cv::Mat gray;
    switch (frame.channels()) {
    case 1:  gray = frame.isContinuous() ? frame : frame.clone(); break;
    case 3:  cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);  break;
    case 4:  cv::cvtColor(frame, gray, cv::COLOR_BGRA2GRAY); break;
    default:
        r.message = "Unsupported image format.";
        return r;
    }

    try {
        ZXing::ImageView view(gray.data, gray.cols, gray.rows,
                              ZXing::ImageFormat::Lum,
                              static_cast<int>(gray.step));

        ZXing::ReaderOptions options;
        options.setFormats(ZXing::BarcodeFormat::Code128 |
                           ZXing::BarcodeFormat::Code39);
        options.setTryHarder(true);

        ZXing::Barcode barcode = ZXing::ReadBarcode(view, options);

        if (!barcode.isValid()) {
            r.message = "No barcode detected.";
            return r;
        }

        const std::string text = barcode.text();
        if (!isValidSixDigitCode(text)) {
            r.message = "Barcode found but its content is not a valid six-digit code.";
            return r;
        }

        r.success = true;
        r.value = text;
        r.message = "Scanned successfully.";
        return r;
    }
    catch (const std::exception& e) {
        r.message = std::string("Decoder error: ") + e.what();
        return r;
    }
}
