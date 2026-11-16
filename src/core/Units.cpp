#include "Units.h"

namespace {
// "125.50" -> 12550 (scale 2) ya "1.25" -> 1250 (scale 3). Sirf digits aur ek '.'.
bool parseScaled(const std::string& s, int maxDecimals, int scale, std::int64_t& out)
{
    std::string ip, fp;
    bool dot = false;
    for (char c : s) {
        if (c == '.') { if (dot) return false; dot = true; }
        else if (c >= '0' && c <= '9') (dot ? fp : ip).push_back(c);
        else return false;
    }
    if (ip.empty() || ip.size() > 12) return false;
    if (dot && fp.empty()) return false;
    if (static_cast<int>(fp.size()) > maxDecimals) return false;
    while (static_cast<int>(fp.size()) < scale) fp.push_back('0');
    out = std::stoll(ip + fp);
    return true;
}

std::string padLeft(std::int64_t v, int width)
{
    std::string s = std::to_string(v);
    while (static_cast<int>(s.size()) < width) s = "0" + s;
    return s;
}
}

std::string measureUnitToText(MeasureUnit unit)
{
    return unit == MeasureUnit::Count ? "count" : "weight";
}

OperationResult parseMeasureUnit(const std::string& text, MeasureUnit& out)
{
    OperationResult r;
    if (text == "count") out = MeasureUnit::Count;
    else if (text == "weight") out = MeasureUnit::Weight;
    else { r.message = "Unit must be count or weight."; return r; }
    r.success = true;
    return r;
}

OperationResult parseMoney(const std::string& text, Money& out)
{
    OperationResult r;
    std::int64_t v = 0;
    if (!parseScaled(text, 2, 2, v)) { r.message = "Invalid amount '" + text + "'."; return r; }
    out = v;
    r.success = true;
    return r;
}

std::string formatMoney(Money value)
{
    std::string sign = value < 0 ? "-" : "";
    if (value < 0) value = -value;
    return sign + std::to_string(value / 100) + "." + padLeft(value % 100, 2);
}

OperationResult parseQuantity(const std::string& text, MeasureUnit unit, Quantity& out)
{
    OperationResult r;
    std::int64_t v = 0;
    int maxDec = (unit == MeasureUnit::Count) ? 0 : 3;
    if (!parseScaled(text, maxDec, 3, v)) {
        r.message = "Invalid quantity '" + text + "' for a " + measureUnitToText(unit) + " product.";
        return r;
    }
    out = v;
    r.success = true;
    return r;
}

std::string formatQuantity(Quantity value, MeasureUnit unit)
{
    if (unit == MeasureUnit::Count) return std::to_string(value / 1000);
    return std::to_string(value / 1000) + "." + padLeft(value % 1000, 3);
}

Money multiplyPriceByQuantity(Money unitPrice, Quantity quantity)
{
    return (unitPrice * quantity + 500) / 1000;   // rounded half up
}