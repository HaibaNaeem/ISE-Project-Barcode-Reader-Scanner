#pragma once
#include <string>
#include <opencv2/core.hpp>
#include "CommonTypes.h"

// ImageDecoder (Member 3 Service Interface & Adapter)
// Loads an image file using OpenCV and decodes the barcode using ZXing.
// Decoupled from the GUI so Member 3 or tests can use it independently.
class ImageDecoder {
public:
    // Decodes barcode from an image file path (PNG, JPG, JPEG, BMP).
    // Returns OperationResult:
    //   - success = true if a valid 6-digit barcode is found.
    //   - value = the 6-digit decoded string.
    //   - message = explanation of success or error reason.
    static OperationResult decodeFile(const std::string& filePath);

    // Decodes barcode directly from a loaded OpenCV Mat.
    static OperationResult decodeMat(const cv::Mat& image);
};
