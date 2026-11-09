#include "BarcodeGenerator.h"
#include "CodeValidator.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

namespace {

    // Code 39: har character mein 9 elements (bar, space, bar, ...).
    // 'n' = narrow, 'w' = wide. Har character mein 3 wide hote hain.
    const char* patternFor(char c)
    {
        switch (c) {
        case '0': return "nnnwwnwnn";
        case '1': return "wnnwnnnnw";
        case '2': return "nnwwnnnnw";
        case '3': return "wnwwnnnnn";
        case '4': return "nnnwwnnnw";
        case '5': return "wnnwwnnnn";
        case '6': return "nnwwwnnnn";
        case '7': return "nnnwnnwnw";
        case '8': return "wnnwnnwnn";
        case '9': return "nnwwnnwnn";
        case '*': return "nwnnwnwnn";   // start/stop
        default:  return nullptr;
        }
    }

    const int NARROW = 3;              // narrow element ki width (pixels)
    const int WIDE = 9;              // wide = 3 x narrow
    const int QUIET_ZONE = 10 * NARROW;    // dono taraf khali jagah
    const int BAR_HEIGHT = 150;
    const int MARGIN_TB = 20;             // upar/neeche safed margin

    // true = kala pixel, false = safed
    std::vector<bool> buildModules(const std::string& code)
    {
        std::vector<bool> m;

        auto add = [&m](int width, bool dark) {
            for (int i = 0; i < width; i++)
                m.push_back(dark);
            };

        add(QUIET_ZONE, false);

        const std::string full = "*" + code + "*";
        for (size_t k = 0; k < full.size(); k++) {
            const char* p = patternFor(full[k]);
            for (int i = 0; i < 9; i++) {
                int w = (p[i] == 'w') ? WIDE : NARROW;
                bool dark = (i % 2 == 0);          // even position = bar
                add(w, dark);
            }
            if (k + 1 < full.size())
                add(NARROW, false);                // characters ke beech gap
        }

        add(QUIET_ZONE, false);
        return m;
    }

    void put16(std::vector<unsigned char>& b, uint16_t v)
    {
        b.push_back(v & 0xFF);
        b.push_back((v >> 8) & 0xFF);
    }

    void put32(std::vector<unsigned char>& b, uint32_t v)
    {
        for (int i = 0; i < 4; i++)
            b.push_back((v >> (8 * i)) & 0xFF);
    }

} // namespace

OperationResult generateBarcode(
    const std::string& code,
    const std::string& outputBmpPath)
{
    OperationResult result;

    // 1. Input check
    if (!isValidSixDigitCode(code)) {
        result.message = "Enter exactly six digits (0-9). The first digit must not be 0.";
        return result;
    }

    // 2. Output folder banao
    try {
        std::filesystem::path p(outputBmpPath);
        if (p.has_parent_path())
            std::filesystem::create_directories(p.parent_path());
    }
    catch (const std::exception& e) {
        result.message = std::string("Cannot create output folder: ") + e.what();
        return result;
    }

    // 3. Bars banao
    std::vector<bool> modules = buildModules(code);
    const int width = static_cast<int>(modules.size());
    const int height = BAR_HEIGHT + 2 * MARGIN_TB;

    // 4. 24-bit BMP banao
    const int rowBytes = ((width * 3 + 3) / 4) * 4;   // row 4 bytes ke multiple mein
    const uint32_t imageSize = static_cast<uint32_t>(rowBytes) * height;

    std::vector<unsigned char> buf;
    buf.reserve(54 + imageSize);

    // File header (14 bytes)
    buf.push_back('B'); buf.push_back('M');
    put32(buf, 54 + imageSize);
    put32(buf, 0);
    put32(buf, 54);

    // Info header (40 bytes)
    put32(buf, 40);
    put32(buf, static_cast<uint32_t>(width));
    put32(buf, static_cast<uint32_t>(height));
    put16(buf, 1);
    put16(buf, 24);
    put32(buf, 0);
    put32(buf, imageSize);
    put32(buf, 2835);
    put32(buf, 2835);
    put32(buf, 0);
    put32(buf, 0);

    // Pixels (upar/neeche margin barabar hai, isliye row order ka farq nahi)
    for (int y = 0; y < height; y++) {
        bool inBarArea = (y >= MARGIN_TB && y < height - MARGIN_TB);
        for (int x = 0; x < width; x++) {
            unsigned char v = (inBarArea && modules[x]) ? 0 : 255;
            buf.push_back(v); buf.push_back(v); buf.push_back(v);
        }
        for (int pad = width * 3; pad < rowBytes; pad++)
            buf.push_back(0);
    }

    // 5. File mein likho
    std::ofstream out(outputBmpPath, std::ios::binary);
    if (!out) {
        result.message = "Cannot open output file for writing.";
        return result;
    }
    out.write(reinterpret_cast<const char*>(buf.data()),
        static_cast<std::streamsize>(buf.size()));
    out.close();
    if (!out) {
        result.message = "Failed to write the image file.";
        return result;
    }

    result.success = true;
    result.message = "Barcode generated successfully.";
    result.outputPath = outputBmpPath;
    return result;
}