// OWNER: Member 2. STUB written by Member 5 so the project builds. Replace with the real code.
#include "Units.h"

std::string measureUnitToText(MeasureUnit unit) { return unit == MeasureUnit::Count ? "count" : "weight"; }

OperationResult parseMeasureUnit(const std::string&, MeasureUnit&) { return {false, "TODO (M2): parseMeasureUnit"}; }
OperationResult parseMoney(const std::string&, Money&)             { return {false, "TODO (M2): parseMoney"}; }
std::string formatMoney(Money)                                     { return "TODO"; }
OperationResult parseQuantity(const std::string&, MeasureUnit, Quantity&) { return {false, "TODO (M2): parseQuantity"}; }
std::string formatQuantity(Quantity, MeasureUnit)                  { return "TODO"; }
Money multiplyPriceByQuantity(Money, Quantity)                     { return 0; }
