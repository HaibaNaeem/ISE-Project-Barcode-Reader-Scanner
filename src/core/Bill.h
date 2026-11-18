#pragma once
// OWNER: Member 4. Shared contract (frozen).
#include <string>
#include <vector>
#include "Order.h"

// One row of Bill.txt = one product on the order. All totals are for THIS product.
struct BillLine {
    std::string productId, acronym;
    Money unitPrice = 0, discountPerUnit = 0;      // column order in Bill.txt (see sample row)
    Quantity quantity = 0;
    Money lineSubtotal = 0;   // price x qty
    Money lineTax = 0;        // tax x qty
    Money lineDiscount = 0;   // discount x qty
    Money linePayable = 0;    // subtotal + tax - discount
};

struct Bill {
    std::string id;                    // 9 digits, e.g. 000000031
    std::string customerName, orderId;
    OrderStatus status = OrderStatus::Paid;
    std::vector<BillLine> lines;
    Money subtotal = 0, tax = 0, discount = 0, payable = 0;   // sums of the lines
};

struct PaymentInfo {
    PaymentMode mode = PaymentMode::Cash;
    std::string merchant;              // required for card / wallet, empty for cash
};
