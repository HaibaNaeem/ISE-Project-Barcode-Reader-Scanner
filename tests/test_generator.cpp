#include <iostream>
#include <filesystem>
#include "BarcodeGenerator.h"

static int passed = 0;
static int failed = 0;

static void runTest(const std::string& id, const std::string& input, bool expectSuccess)
{
    std::string name = input.empty() ? "empty" : input;
    std::string path = "output/generated_barcodes/" + name + ".bmp";

    OperationResult r = generateBarcode(input, path);

    bool pass = (r.success == expectSuccess);
    if (pass && expectSuccess)
        pass = std::filesystem::exists(path) && std::filesystem::file_size(path) > 0;

    std::cout << id << " | input=\"" << input << "\" | "
        << (pass ? "PASS" : "FAIL") << " | " << r.message << "\n";

    if (pass) passed++; else failed++;
}

int main()
{
    // Valid
    runTest("G-01", "123456", true);
    runTest("G-02", "100230", true);
    runTest("G-03", "999999", true);

    // Pehla digit zero: ab invalid
    runTest("V-01", "000123", false);
    runTest("V-02", "012345", false);

    // Baqi invalid
    runTest("V-03a", "12345", false);
    runTest("V-03b", "1234567", false);
    runTest("V-04", "12A456", false);
    runTest("V-05", "", false);
    runTest("V-06", "12 456", false);

    std::cout << "\nPassed: " << passed << "  Failed: " << failed << "\n";
    return failed == 0 ? 0 : 1;
}