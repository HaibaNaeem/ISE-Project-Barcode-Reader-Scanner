#include "BarcodeDecoder.h"
#include "CodeValidator.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <ZXing/ReadBarcode.h>

using namespace std;

OperationResult decodeBarcodeImage(const string& imagePath)
{
    OperationResult r;

    if (imagePath.empty()) {
        r.message = "No image file selected.";
        return r;
    }

    cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);
    if (image.empty()) {
        r.message = "Image file could not be opened (missing, corrupt or unsupported).";
        return r;
    }

    return decodeBarcodeMat(image);
}

OperationResult decodeBarcodeMat(const cv::Mat& image)
{
    OperationResult r;

    if (image.empty()) {
        r.message = "Empty image.";
        return r;
    }

    // ZXing needs grayscale 
    cv::Mat gray;
    switch (image.channels()) {
    case 1:  gray = image.isContinuous() ? image : image.clone(); break;
    case 3:  cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);  break;
    case 4:  cv::cvtColor(image, gray, cv::COLOR_BGRA2GRAY); break;
    default:
        r.message = "Unsupported image format.";
        return r;
    }

    try {
        ZXing::ImageView view(gray.data, gray.cols, gray.rows,
                              ZXing::ImageFormat::Lum,
                              static_cast<int>(gray.step));

        ZXing::ReaderOptions options;
        options.setFormats(ZXing::BarcodeFormat::Code39);
        options.setTryHarder(true);

        ZXing::Barcode barcode = ZXing::ReadBarcode(view, options);

        if (!barcode.isValid()) {
            r.message = "No barcode detected.";
            return r;
        }

        const string text = barcode.text();
        if (!isValidSixDigitCode(text)) {
            r.message = "Barcode found but its content is not a valid six-digit code.";
            return r;
        }

        r.success = true;
        r.value = text;
        r.message = "Decoded successfully.";
        return r;
    }
    catch (const exception& e) {
        r.message = string("Decoder error: ") + e.what();
        return r;
    }
}
