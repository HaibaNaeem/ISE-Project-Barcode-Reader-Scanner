#pragma once
#include <string>
#include "CommonTypes.h"

// code          : 6-digit string, pehla digit 1-9
// outputBmpPath : jaise "output/generated_barcodes/123456.bmp"
OperationResult generateBarcode(
    const std::string& code,
    const std::string& outputBmpPath
);