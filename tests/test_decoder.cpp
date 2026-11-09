#include <iostream>
#include <filesystem>
#include "ImageDecoder.h"
#include "BarcodeGenerator.h"

static int passed = 0;
static int failed = 0;

static void runDecoderTest(const std::string& id, const std::string& filePath, bool expectSuccess, const std::string& expectedValue = "")
{
    OperationResult r = ImageDecoder::decodeFile(filePath);

    bool pass = (r.success == expectSuccess);
    if (pass && expectSuccess && !expectedValue.empty()) {
        pass = (r.value == expectedValue);
    }

    std::cout << id << " | file=\"" << filePath << "\" | "
              << (pass ? "PASS" : "FAIL") << " | "
              << "result=\"" << r.value << "\" | "
              << r.message << "\n";

    if (pass) passed++; else failed++;
}

int main()
{
    std::cout << "=== Image Decoder Service Tests (Member 3 Adapter) ===\n\n";

    // 1. Ensure test barcode exists (generate one if missing)
    std::string testBmp = "output/generated_barcodes/123456.bmp";
    if (!std::filesystem::exists(testBmp)) {
        generateBarcode("123456", testBmp);
    }

    // Test valid barcodes
    if (std::filesystem::exists("output/generated_barcodes/123456.bmp")) {
        runDecoderTest("D-01", "output/generated_barcodes/123456.bmp", true, "123456");
    }
    if (std::filesystem::exists("output/generated_barcodes/100230.bmp")) {
        runDecoderTest("D-02", "output/generated_barcodes/100230.bmp", true, "100230");
    }

    // Test non-existent file
    runDecoderTest("D-03", "non_existent_file.png", false);

    // Test empty path
    runDecoderTest("D-04", "", false);

    std::cout << "\nDecoder Tests Passed: " << passed << "  Failed: " << failed << "\n";
    return failed == 0 ? 0 : 1;
}
