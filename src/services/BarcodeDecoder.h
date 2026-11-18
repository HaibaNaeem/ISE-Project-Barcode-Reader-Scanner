#pragma once
#include <string>
#include <opencv2/core.hpp>
#include "CommonTypes.h"

OperationResult decodeBarcodeImage(const std::string& imagePath);
OperationResult decodeBarcodeMat(const cv::Mat& image);