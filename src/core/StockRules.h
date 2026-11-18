#pragma once
// OWNER: Member 3. The ONLY place that decides if stock is enough (section 4 of the team plan).
#include "Units.h"

// Accepted if (already in the order + requested) <= available. Selling the last unit is allowed.
// If the instructor says "less than", change ONLY the <= below.
namespace StockRules {
inline bool hasEnoughStock(Quantity inCart, Quantity requested, Quantity available) {
    if (requested < 0 || inCart < 0) {
        return false;
    }
    return inCart + requested <= available;
}
}

using StockRules::hasEnoughStock;
