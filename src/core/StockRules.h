#pragma once
#include <cstdint>

using Quantity = std::int64_t;

namespace StockRules {

// Check if adding requested quantity to cart is allowed given available stock.
// Rule: accepted if (inCart + requested) <= available.
// Selling the last unit is allowed.
inline bool hasEnoughStock(Quantity inCart, Quantity requested, Quantity available) {
    if (requested < 0 || inCart < 0) {
        return false;
    }
    return (inCart + requested) <= available;
}

} // namespace StockRules
