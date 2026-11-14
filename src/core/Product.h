#pragma once
// OWNER: Member 2. Shared contract (frozen).
#include <string>
#include "Units.h"

struct Product {
    std::string id, barcode;
    std::string acronym;               // max 8 characters
    std::string name;
    MeasureUnit unit = MeasureUnit::Count;
    Money unitPrice = 0, taxPerUnit = 0, discountPerUnit = 0;   // fixed PKR per unit (paisa)
    Quantity available = 0;                                     // stock, thousandths
    std::string barcodeImagePath;      // relative to the folder of the .exe, e.g. sample_data/barcodes/123456.bmp
};
