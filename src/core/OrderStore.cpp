#include "OrderStore.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

// TODO(M3, after Member 2): Implement file I/O using RecordFormat/Units
// Format: Order.txt: 00000714 2026-11-16 14:05:09 "Ali Khan" "" on-hold 0.00 none "" #
//         OrderDetail.txt: 00000714 101 123456 2 #
// Text with spaces or empty text in quotes; numbers/codes without quotes; every record ends with #; money/quantity text via Units.
OperationResult OrderStore::save(const Order& order) {
    return OperationResult{
        false,
        "OrderStore file I/O not implemented yet (waiting for Member 2 RecordFormat/Units)",
        "",
        ""
    };
}

OperationResult OrderStore::load(std::vector<Order>& orders) {
    return OperationResult{
        false,
        "OrderStore file I/O not implemented yet (waiting for Member 2 RecordFormat/Units)",
        "",
        ""
    };
}

// Pure function: generate next order ID
// 8 digits = 6-digit zero-padded counter (largest counter in ALL existing orders + 1) + 2-digit day of month
// Example: 00000714 = counter 7, 14th day of month
std::string OrderStore::generateNextOrderId(const std::vector<Order>& existingOrders, int dayOfMonth) {
    int maxCounter = 0;

    for (const auto& order : existingOrders) {
        if (order.id.length() == 8) {
            // Extract counter from first 6 digits
            std::string counterStr = order.id.substr(0, 6);
            try {
                int counter = std::stoi(counterStr);
                if (counter > maxCounter) {
                    maxCounter = counter;
                }
            } catch (...) {
                // Invalid ID format, skip
            }
        }
    }

    int nextCounter = maxCounter + 1;

    // Format: 6-digit zero-padded counter + 2-digit day
    std::stringstream ss;
    ss << std::setw(6) << std::setfill('0') << nextCounter;
    ss << std::setw(2) << std::setfill('0') << dayOfMonth;

    return ss.str();
}

// Crash recovery: turn every in-progress order into on-hold
void OrderStore::recoverInProgressOrders(std::vector<Order>& orders) {
    for (auto& order : orders) {
        if (order.status == OrderStatus::InProgress) {
            order.status = OrderStatus::OnHold;
        }
    }
}

// Orphan check helper: given orders and detail lines (as strings), return lines whose order id does not exist
// Detail line format expected: "ORDERID ..." (first token is order ID)
std::vector<std::string> OrderStore::findOrphanDetailLines(const std::vector<Order>& orders,
                                                           const std::vector<std::string>& detailLines) {
    std::vector<std::string> orphanLines;

    // Build set of existing order IDs
    std::vector<std::string> existingIds;
    for (const auto& order : orders) {
        existingIds.push_back(order.id);
    }

    for (const auto& line : detailLines) {
        std::stringstream ss(line);
        std::string orderId;
        if (ss >> orderId) {
            // Check if this order ID exists
            bool found = false;
            for (const auto& id : existingIds) {
                if (id == orderId) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                orphanLines.push_back(line);
            }
        }
    }

    return orphanLines;
}
