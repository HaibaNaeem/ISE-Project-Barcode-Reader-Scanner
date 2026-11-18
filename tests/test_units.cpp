#include <iostream>
#include "Units.h"

static int failed = 0;
#define CHECK(c) do { if (!(c)) { std::cout << "FAIL line " << __LINE__ << ": " #c "\n"; ++failed; } } while (0)

static bool money(const char* t, Money expect) { Money m = -1; return parseMoney(t, m).success && m == expect; }
static bool badMoney(const char* t) { Money m; return !parseMoney(t, m).success; }
static bool qty(const char* t, MeasureUnit u, Quantity expect) { Quantity q = -1; return parseQuantity(t, u, q).success && q == expect; }
static bool badQty(const char* t, MeasureUnit u) { Quantity q; return !parseQuantity(t, u, q).success; }

int main()
{
    CHECK(money("125.50", 12550));
    CHECK(money("125.5", 12550));
    CHECK(money("0.05", 5));
    CHECK(money("0.00", 0));
    CHECK(badMoney("-1.00"));
    CHECK(badMoney("1.234"));
    CHECK(badMoney("abc"));
    CHECK(badMoney(""));
    CHECK(formatMoney(12550) == "125.50");
    CHECK(formatMoney(5) == "0.05");

    CHECK(qty("12", MeasureUnit::Count, 12000));
    CHECK(badQty("1.5", MeasureUnit::Count));
    CHECK(qty("1.25", MeasureUnit::Weight, 1250));
    CHECK(badQty("1.2345", MeasureUnit::Weight));
    CHECK(badQty("-2", MeasureUnit::Count));
    CHECK(formatQuantity(25500, MeasureUnit::Weight) == "25.500");
    CHECK(formatQuantity(3000, MeasureUnit::Count) == "3");

    CHECK(multiplyPriceByQuantity(45000, 2000) == 90000);
    CHECK(multiplyPriceByQuantity(10000, 3000) == 30000);
    CHECK(multiplyPriceByQuantity(16000, 1250) == 20000);
    CHECK(multiplyPriceByQuantity(999, 500) == 500);
    CHECK(multiplyPriceByQuantity(10000, 0) == 0);

    MeasureUnit u;
    CHECK(parseMeasureUnit("count", u).success && u == MeasureUnit::Count);
    CHECK(parseMeasureUnit("weight", u).success && u == MeasureUnit::Weight);
    CHECK(!parseMeasureUnit("liters", u).success);

    std::cout << (failed ? "FAILED\n" : "All unit tests passed\n");
    return failed ? 1 : 0;
}