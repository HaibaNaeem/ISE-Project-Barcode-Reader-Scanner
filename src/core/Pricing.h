#pragma once
// OWNER: Member 4. Pure calculation, no files. The GUI never does money maths: it calls these.
#include <string>
#include "Bill.h"
#include "ProductCatalog.h"

// Price one line. Check: price 100, tax 15, discount 10, qty 3 -> 300 / 45 / 30 / payable 315.
BillLine priceLine(const Product& product, Quantity quantity);

// Build the whole bill for an order. Lines whose product is not in the catalog are skipped
// (CheckoutService rejects such orders before this).
Bill buildBill(const Order& order, const ProductCatalog& catalog, const std::string& billId);

// Change to give back for a cash payment: tendered - payable (0 if tendered is less).
Money changeDue(Money payable, Money tendered);
