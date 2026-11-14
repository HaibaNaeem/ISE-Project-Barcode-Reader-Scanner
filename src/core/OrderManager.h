#pragma once
// OWNER: Member 3. Shared contract (frozen). The order being recorded + held orders.
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include "CommonTypes.h"
#include "Order.h"
#include "OrderStore.h"
#include "ProductCatalog.h"

class OrderManager {
public:
    // store may be nullptr: then nothing is saved (use this in unit tests).
    OrderManager(const ProductCatalog& catalog, OrderStore* store = nullptr);

    OperationResult startOrder(const std::string& customerName, const std::string& customerContact);
    OperationResult addByBarcode(const std::string& barcode, Quantity quantity);  // merges repeated scans, checks stock
    OperationResult setQuantity(const std::string& productId, Quantity quantity);
    OperationResult removeLine(const std::string& productId);
    OperationResult hold();
    OperationResult resume(const std::string& orderId);
    OperationResult cancel();                                  // status archived, stock unchanged

    const std::optional<Order>& current() const { return current_; }
    std::vector<Order> heldOrders() const;

    // Used by CheckoutService after a successful payment: forget the current order.
    void clearCurrent() { current_.reset(); }

    // Tests can fix the date so Order IDs are predictable.
    void setClock(std::function<DateTime()> clock) { clock_ = std::move(clock); }

private:
    const ProductCatalog* catalog_;
    OrderStore* store_;
    std::optional<Order> current_;
    std::function<DateTime()> clock_;
};
