#define _CRT_SECURE_NO_WARNINGS
#include "BarcodeGenerator.h"
#include "CodeValidator.h"

#include <zint.h>
#include <cstring>
#include <filesystem>

OperationResult generateCode128(
    const std::string& code,
    const std::string& outputPngPath)
{
    OperationResult result;

    // 1. Input check
    if (!isValidSixDigitCode(code)) {
        result.message = "Enter exactly six digits (0-9 only).";
        return result;
    }

    // 2. Output folder bana do
    try {
        std::filesystem::path p(outputPngPath);
        if (p.has_parent_path())
            std::filesystem::create_directories(p.parent_path());
    }
    catch (const std::exception& e) {
        result.message = std::string("Cannot create output folder: ") + e.what();
        return result;
    }

    // 3. Zint object
    zint_symbol* symbol = ZBarcode_Create();
    if (symbol == nullptr) {
        result.message = "Failed to initialize barcode generator.";
        return result;
    }

    // 4. Settings
    symbol->symbology = BARCODE_CODE128;
    symbol->scale = 2.0f;
    symbol->height = 50;
    symbol->whitespace_width = 10;
    std::strncpy(symbol->outfile, outputPngPath.c_str(), sizeof(symbol->outfile) - 1);
    symbol->outfile[sizeof(symbol->outfile) - 1] = '\0';

    // 5. Barcode banao aur file mein likho
    int err = ZBarcode_Encode_and_Print(
        symbol,
        reinterpret_cast<const unsigned char*>(code.c_str()),
        static_cast<int>(code.size()),
        0
    );

    // 6. Result
    if (err >= ZINT_ERROR) {
        result.message = std::string("Barcode generation failed: ") + symbol->errtxt;
    }
    else {
        result.success = true;
        result.message = "Barcode generated successfully.";
        result.outputPath = outputPngPath;
    }

    // 7. Memory free
    ZBarcode_Delete(symbol);
    return result;
}