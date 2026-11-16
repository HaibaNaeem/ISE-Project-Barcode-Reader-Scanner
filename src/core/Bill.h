#pragma once
// OWNER: Member 4. Shared contract for Increment 2 billing data structures.
#include <string>
#include <vector>
#include "Order.h"

// One row of Bill.txt = one product on the order. All totals are for this single product.
struct BillLine {
    std::string productId;
    std::string acronym;
    Money unitPrice = 0;           // Base price without tax (paisa)
    Money discountPerUnit = 0;     // Fixed PKR discount per unit (paisa)
    Quantity quantity = 0;         // In thousandths (e.g. 2 Count = 2000, 1.250 Weight = 1250)
    Money lineSubtotal = 0;        // Base price x quantity (without tax)
    Money lineTax = 0;             // Tax x quantity
    Money lineDiscount = 0;        // Discount x quantity
    Money linePayable = 0;         // Subtotal + Tax - Discount
};

// Represents a complete customer bill containing one or more product lines.
struct Bill {
    std::string id;                                     // 9 digits, e.g. "000000031"
    std::string customerName;
    std::string orderId;                                // 8 digits, e.g. "00000714"
    std::string date;                                   // e.g. "2026-11-16"
    std::string time;                                   // e.g. "14:05:09"
    OrderStatus status = OrderStatus::Paid;             // Reuses OrderStatus enum from Order.h
    std::vector<BillLine> lines;                        // Individual product lines
    Money subtotal = 0;                                 // Grand subtotal without tax
    Money tax = 0;                                      // Grand total tax
    Money discount = 0;                                 // Grand total discount
    Money payable = 0;                                  // Grand total payable
    PaymentMode paymentMode = PaymentMode::Cash;        // Cash, Card, or Wallet
    std::string merchant;                               // Merchant name for card/wallet; empty for cash
};

// Payment information supplied by cashier during checkout.
struct PaymentInfo {
    PaymentMode mode = PaymentMode::Cash;
    std::string merchant;                               // Required for Card and Wallet; must be empty for Cash
};
