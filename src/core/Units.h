#pragma once
// OWNER: Member 2. Shared contract (frozen). Do not change signatures without telling the team.
#include <cstdint>
#include <string>
#include "CommonTypes.h"

using Money    = std::int64_t;  // paisa: 12550 = 125.50 PKR
using Quantity = std::int64_t;  // thousandths: 3 pcs = 3000, 1.25 kg = 1250

enum class MeasureUnit { Count, Weight };

// "count" <-> "weight"
std::string measureUnitToText(MeasureUnit unit);
OperationResult parseMeasureUnit(const std::string& text, MeasureUnit& out);

// "125.50" -> 12550. Rejects negatives, letters, more than 2 decimals.
OperationResult parseMoney(const std::string& text, Money& out);
std::string formatMoney(Money value);                       // 12550 -> "125.50"

// Count: whole numbers only ("3" -> 3000). Weight: up to 3 decimals ("1.25" -> 1250).
OperationResult parseQuantity(const std::string& text, MeasureUnit unit, Quantity& out);
std::string formatQuantity(Quantity value, MeasureUnit unit); // 3000 Count -> "3"; 1250 Weight -> "1.250"

// price x quantity with rounding (half up). 450.00 x 2 pcs -> 90000 paisa.
// Quantity is in thousandths, so result = price * quantity / 1000, rounded.
Money multiplyPriceByQuantity(Money unitPrice, Quantity quantity);
