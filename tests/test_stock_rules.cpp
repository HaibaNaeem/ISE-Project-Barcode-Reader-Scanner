#include "../src/core/StockRules.h"
#include "TestHelpers.h"
#include <iostream>

using Quantity = std::int64_t;

int main() {
    // Stock 5: 0+5 ok, 0+6 no, 3+2 ok, 3+3 no, exact stock ok, requested 0 and negative handled
    CHECK(StockRules::hasEnoughStock(0, 5, 5), "0+5 <= 5 (exact stock ok)");
    CHECK(!StockRules::hasEnoughStock(0, 6, 5), "0+6 > 5 (rejected)");
    CHECK(StockRules::hasEnoughStock(3, 2, 5), "3+2 <= 5 (ok)");
    CHECK(!StockRules::hasEnoughStock(3, 3, 5), "3+3 > 5 (rejected)");
    CHECK(StockRules::hasEnoughStock(0, 5, 5), "exact stock allowed");
    CHECK(StockRules::hasEnoughStock(0, 0, 5), "requested 0 allowed");
    CHECK(!StockRules::hasEnoughStock(0, -1, 5), "negative requested rejected");
    CHECK(StockRules::hasEnoughStock(5, 0, 5), "selling last unit allowed");

    std::cout << "\nPassed: " << passed << "  Failed: " << failed << "\n";
    return failed == 0 ? 0 : 1;
}
