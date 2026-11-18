#pragma once
// OWNER: Member 3. Order.txt + OrderDetail.txt are always loaded and saved together.
#include <string>
#include <vector>
#include "CommonTypes.h"
#include "Order.h"

class OrderStore {
public:
    OrderStore() = default;
    OrderStore(std::string orderPath, std::string detailPath);

    OperationResult load();                      // in-progress orders become on-hold
    OperationResult save() const;                // writes both files (not used by pay(); see serialize*)

    std::string serializeOrders() const;         // whole Order.txt as text (used by CheckoutService)
    std::string serializeDetails() const;        // whole OrderDetail.txt as text
    const std::string& orderPath() const  { return orderPath_; }
    const std::string& detailPath() const { return detailPath_; }

    // 8 digits: 6-digit counter (last used + 1) + day of month. e.g. 00000714. Never repeats after restart.
    std::string nextOrderId(int dayOfMonth) const;

    std::vector<Order>& orders() { return orders_; }
    const std::vector<Order>& orders() const { return orders_; }

private:
    std::string orderPath_, detailPath_;
    std::vector<Order> orders_;
};
