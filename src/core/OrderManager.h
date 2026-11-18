#pragma once
#include "Order.h"
#include "ProductCatalog.h"
#include "OrderStore.h"
#include "../services/CommonTypes.h"
#include <optional>
#include <vector>
#include <string>
#include <functional>

// Forward declaration for OrderStore (optional dependency)
class OrderStore;

class OrderManager {
public:
    // Constructor with injectable date/time function for deterministic tests
    // Default uses system clock
    OrderManager(const ProductCatalog& catalog,
                 OrderStore* store = nullptr,
                 std::function<std::string()> dateTimeProvider = nullptr);

    OperationResult startOrder(const std::string& name, const std::string& contact);
    OperationResult addByBarcode(const std::string& barcode, Quantity qty);
    OperationResult setQuantity(const std::string& productId, Quantity qty);
    OperationResult removeLine(const std::string& productId);
    OperationResult hold();
    OperationResult resume(const std::string& orderId);
    OperationResult cancel();

    const std::optional<Order>& current() const;
    std::vector<Order> heldOrders() const;

private:
    // Pure function to generate next order ID (delegates to OrderStore helper)
    std::string generateOrderId(int dayOfMonth) const;

    // Helper to get current date/time (uses injectable function or system clock)
    std::string getCurrentDateTime() const;

    const ProductCatalog& catalog_;
    OrderStore* store_;  // Optional: may be null, skip saving if null
    std::optional<Order> currentOrder_;
    std::vector<Order> allOrders_;  // Keeps all orders (held/archived) in memory
    std::function<std::string()> dateTimeProvider_;
};
