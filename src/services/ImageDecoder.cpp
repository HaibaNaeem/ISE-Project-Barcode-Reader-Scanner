#include "ImageDecoder.h"
#include "CameraScanner.h"
#include <opencv2/imgcodecs.hpp>
#include <filesystem>

OperationResult ImageDecoder::decodeFile(const std::string& filePath)
{
    OperationResult result;

    if (filePath.empty()) {
        result.message = "No file path provided.";
        return result;
    }

    try {
        if (!std::filesystem::exists(filePath)) {
            result.message = "File does not exist: " + filePath;
            return result;
        }
    }
    catch (const std::exception& e) {
        result.message = std::string("Filesystem error: ") + e.what();
        return result;
    }

    // Load image using OpenCV (supports PNG, JPG, JPEG, BMP)
    cv::Mat image = cv::imread(filePath, cv::IMREAD_COLOR);
    if (image.empty()) {
        result.message = "Failed to load image. The file may be corrupt or in an unsupported format.";
        return result;
    }

    return decodeMat(image);
}

OperationResult ImageDecoder::decodeMat(const cv::Mat& image)
{
    if (image.empty()) {
        OperationResult result;
        result.message = "Provided image is empty.";
        return result;
    }

    // Delegate to shared decoding engine in CameraScanner
    return CameraScanner::decodeFrame(image);
}
