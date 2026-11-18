#pragma once
#include "Order.h"
#include "../services/CommonTypes.h"
#include <vector>
#include <string>

class OrderStore {
public:
    OrderStore() = default;
    virtual ~OrderStore() = default;

    // TODO(M3, after Member 2): Implement file I/O using RecordFormat/Units
    // Format: Order.txt: 00000714 2026-11-16 14:05:09 "Ali Khan" "" on-hold 0.00 none "" #
    //         OrderDetail.txt: 00000714 101 123456 2 #
    // Text with spaces or empty text in quotes; numbers/codes without quotes; every record ends with #; money/quantity text via Units.
    virtual OperationResult save(const Order& order);
    virtual OperationResult load(std::vector<Order>& orders);

    // Pure functions (implement now, no files, no Units, no RecordFormat)
    
    // Generate next order ID: 8 digits = 6-digit zero-padded counter (largest counter in ALL existing orders + 1) + 2-digit day of month
    // Example: 00000714 = counter 7, 14th day of month
    static std::string generateNextOrderId(const std::vector<Order>& existingOrders, int dayOfMonth);

    // Crash recovery: turn every in-progress order into on-hold
    static void recoverInProgressOrders(std::vector<Order>& orders);

    // Orphan check helper: given orders and detail lines (as strings), return lines whose order id does not exist
    // (to be skipped with a warning later when file I/O is implemented)
    static std::vector<std::string> findOrphanDetailLines(const std::vector<Order>& orders,
                                                           const std::vector<std::string>& detailLines);
};
