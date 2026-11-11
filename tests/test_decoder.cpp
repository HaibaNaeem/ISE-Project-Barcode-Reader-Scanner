#include "BarcodeDecoder.h"
#include <opencv2/imgcodecs.hpp>
#include <iostream>
#include <fstream>

using namespace std;

static int passed = 0, failed = 0, skipped = 0;

static void check(const string& name, bool ok)
{
    cout << (ok ? "[PASS] " : "[FAIL] ") << name << "\n";
    ok ? ++passed : ++failed;
}

static void checkRejected(const string& name, const string& path)
{
    ifstream file(path);

    if (!file.good()) {
        cout << "[SKIP] " << name << " - test image missing\n";
        ++skipped;
        return;
    }

    OperationResult r = decodeBarcodeImage(path);
    check(name, !r.success);
}

int main()
{
    // 1. Valid barcode tests
    for (string code : { "123456", "100230", "999999" }) {

        string path = "output/generated_barcodes/" + code + ".bmp";

        OperationResult r = decodeBarcodeImage(path);

        check(code, r.success && r.value == code);

        if (!r.success)
            cout << "   " << r.message << "\n";
    }

    // 2. Missing file test
    OperationResult missing =
        decodeBarcodeImage("does_not_exist.png");

    check("Missing file rejected", !missing.success);

    // 3. Empty image path test
    OperationResult emptyPath = decodeBarcodeImage("");

    check("Empty image path rejected", !emptyPath.success);

    // 4. Corrupt image test
    ofstream corrupt("corrupt_test.png");
    corrupt << "This is not a real image";
    corrupt.close();

    OperationResult badImage =
        decodeBarcodeImage("corrupt_test.png");

    check("Corrupt image rejected", !badImage.success);

    // 5. Blank image test
    cv::Mat blank(200, 400, CV_8UC3,
                  cv::Scalar(255, 255, 255));

    bool saved = cv::imwrite("blank_test.png", blank);

    if (saved) {
        OperationResult none =
            decodeBarcodeImage("blank_test.png");

        check("No barcode in blank image",
              !none.success && none.value.empty());
    }
    else {
        check("Blank image could not be saved", false);
    }

    // 6. Empty Mat test
    cv::Mat emptyImage;

    OperationResult empty =
        decodeBarcodeMat(emptyImage);

    check("Empty image rejected", !empty.success);

    // 7. JPG image test
    cv::Mat source = cv::imread(
        "output/generated_barcodes/123456.bmp");

    if (!source.empty() &&
        cv::imwrite("jpg_test.jpg", source)) {

        OperationResult jpg =
            decodeBarcodeImage("jpg_test.jpg");

        check("JPG barcode decoding",
              jpg.success && jpg.value == "123456");
    }
    else {
        cout << "[SKIP] JPG test - source BMP missing\n";
        ++skipped;
    }

    // 8. Invalid barcode content tests
    checkRejected(
        "Seven-digit barcode rejected",
        "output/test_barcodes/1234567.bmp"
    );

    checkRejected(
        "Zero-starting barcode rejected",
        "output/test_barcodes/012345.bmp"
    );

    cout << "\n-----------------------------\n";
    cout << "Passed: " << passed << "\n";
    cout << "Failed: " << failed << "\n";
    cout << "Skipped: " << skipped << "\n";
    cout << "-----------------------------\n";

    return failed == 0 ? 0 : 1;
}