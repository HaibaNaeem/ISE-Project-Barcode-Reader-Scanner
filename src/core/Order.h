#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <optional>

using Money = std::int64_t;
using Quantity = std::int64_t;

enum class OrderStatus {
    InProgress,
    OnHold,
    Paid,
    Archived
};

enum class PaymentMode {
    Cash,
    Card,
    Wallet,
    None
};

// Helpers for OrderStatus text <-> enum
inline std::string orderStatusToString(OrderStatus status) {
    switch (status) {
        case OrderStatus::InProgress: return "in-progress";
        case OrderStatus::OnHold: return "on-hold";
        case OrderStatus::Paid: return "paid";
        case OrderStatus::Archived: return "archived";
        default: return "unknown";
    }
}

inline std::optional<OrderStatus> stringToOrderStatus(const std::string& text) {
    if (text == "in-progress") return OrderStatus::InProgress;
    if (text == "on-hold") return OrderStatus::OnHold;
    if (text == "paid") return OrderStatus::Paid;
    if (text == "archived") return OrderStatus::Archived;
    return std::nullopt;
}

// Helpers for PaymentMode text <-> enum
inline std::string paymentModeToString(PaymentMode mode) {
    switch (mode) {
        case PaymentMode::Cash: return "cash";
        case PaymentMode::Card: return "card";
        case PaymentMode::Wallet: return "wallet";
        case PaymentMode::None: return "none";
        default: return "unknown";
    }
}

inline std::optional<PaymentMode> stringToPaymentMode(const std::string& text) {
    if (text == "cash") return PaymentMode::Cash;
    if (text == "card") return PaymentMode::Card;
    if (text == "wallet") return PaymentMode::Wallet;
    if (text == "none") return PaymentMode::None;
    return std::nullopt;
}

struct OrderLine {
    std::string productId;
    std::string barcode;
    Quantity quantity;
};

struct Order {
    std::string id;
    std::string date;
    std::string time;
    std::string customerName;
    std::string customerContact;
    OrderStatus status;
    Money paymentAmount;
    PaymentMode mode;
    std::string merchant;
    std::vector<OrderLine> lines;
};
