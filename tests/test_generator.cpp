#include <iostream>
#include <filesystem>
#include "BarcodeGenerator.h"

static int passed = 0;
static int failed = 0;

static void runTest(const std::string& id, const std::string& input, bool expectSuccess)
{
    std::string name = input.empty() ? "empty" : input;
    std::string path = "output/generated_barcodes/" + name + ".png";

    OperationResult r = generateCode128(input, path);

    bool pass = (r.success == expectSuccess);
    if (pass && expectSuccess)
        pass = std::filesystem::exists(path);

    std::cout << id << " | input=\"" << input << "\" | "
        << (pass ? "PASS" : "FAIL") << " | " << r.message << "\n";

    if (pass) passed++; else failed++;
}

int main()
{
    runTest("G-01", "123456", true);
    runTest("G-02", "000123", true);
    runTest("G-03a", "12345", false);
    runTest("G-03b", "1234567", false);
    runTest("G-04", "12A456", false);
    runTest("G-05", "", false);
    runTest("G-06", "12 456", false);

    std::cout << "\nPassed: " << passed << "  Failed: " << failed << "\n";
    return failed == 0 ? 0 : 1;
}