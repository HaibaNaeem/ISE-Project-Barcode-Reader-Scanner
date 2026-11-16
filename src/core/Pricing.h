#pragma once
// OWNER: Member 4. Pure calculation, no files, no Qt.
// All financial math for pricing lines, calculating bills, and change due.
#include <string>
#include "Bill.h"
#include "ProductCatalog.h"

// Price one line.
// Formula: subtotal = price x qty, tax = tax x qty, discount = discount x qty, payable = subtotal + tax - discount.
// Example: price 100, tax 15, discount 10, qty 3 -> 300 / 45 / 30 / payable 315.
BillLine priceLine(const Product& product, Quantity quantity);

// Build the complete bill for an order with all line calculations and grand totals.
// If a product is not found in the catalog, it is handled gracefully without crashing.
Bill buildBill(const Order& order, const ProductCatalog& catalog, const std::string& billId);

// Change to give back for a cash payment: tendered - payable (returns 0 if tendered <= payable).
Money changeDue(Money payable, Money tendered);

// Convenient alias matching contract terminology
inline Money calculateChange(Money payable, Money tendered) {
    return changeDue(payable, tendered);
}
