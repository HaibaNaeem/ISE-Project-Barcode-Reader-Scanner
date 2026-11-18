#pragma once
// OWNER: Member 3. Shared contract (frozen). Money/Quantity come from Units.h.
#include <string>
#include <vector>
#include <optional>
#include "Units.h"

enum class OrderStatus { InProgress, OnHold, Paid, Archived };
enum class PaymentMode { None, Cash, Card, Wallet };

struct OrderLine { std::string productId, barcode; Quantity quantity = 0; };

struct Order {
    std::string id, date, time, customerName, customerContact;
    OrderStatus status = OrderStatus::InProgress;
    Money paymentAmount = 0;
    PaymentMode mode = PaymentMode::None;
    std::string merchant;
    std::vector<OrderLine> lines;
};

// Text <-> enum, exactly as written in Order.txt.
inline std::string orderStatusToText(OrderStatus s) {
    switch (s) {
        case OrderStatus::InProgress: return "in-progress";
        case OrderStatus::OnHold:     return "on-hold";
        case OrderStatus::Paid:       return "paid";
        case OrderStatus::Archived:   return "archived";
    }
    return "in-progress";
}
inline bool parseOrderStatus(const std::string& t, OrderStatus& out) {
    if (t == "in-progress") { out = OrderStatus::InProgress; return true; }
    if (t == "on-hold")     { out = OrderStatus::OnHold;     return true; }
    if (t == "paid")        { out = OrderStatus::Paid;       return true; }
    if (t == "archived")    { out = OrderStatus::Archived;   return true; }
    return false;
}
inline std::string paymentModeToText(PaymentMode m) {
    switch (m) {
        case PaymentMode::Cash:   return "cash";
        case PaymentMode::Card:   return "card";
        case PaymentMode::Wallet: return "wallet";
        case PaymentMode::None:   return "none";
    }
    return "none";
}
inline bool parsePaymentMode(const std::string& t, PaymentMode& out) {
    if (t == "cash")   { out = PaymentMode::Cash;   return true; }
    if (t == "card")   { out = PaymentMode::Card;   return true; }
    if (t == "wallet") { out = PaymentMode::Wallet; return true; }
    if (t == "none")   { out = PaymentMode::None;   return true; }
    return false;
}

// Aliases for member 3 compatibility
inline std::string orderStatusToString(OrderStatus s) { return orderStatusToText(s); }
inline std::string paymentModeToString(PaymentMode m) { return paymentModeToText(m); }
inline std::optional<OrderStatus> stringToOrderStatus(const std::string& t) {
    OrderStatus s;
    if (parseOrderStatus(t, s)) return s;
    return std::nullopt;
}
inline std::optional<PaymentMode> stringToPaymentMode(const std::string& t) {
    PaymentMode m;
    if (parsePaymentMode(t, m)) return m;
    return std::nullopt;
}

// Date/time used when an order is created. Tests can replace it (OrderManager::setClock).
struct DateTime {
    std::string date;   // 2026-11-16
    std::string time;   // 14:05:09 (24-hour)
    int dayOfMonth = 1; // 16 -> last two digits of the Order ID
};
