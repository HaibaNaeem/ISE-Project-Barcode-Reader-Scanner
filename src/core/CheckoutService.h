#pragma once
// OWNER: Member 4. Payment is all or nothing.
#include <string>
#include "CommonTypes.h"
#include "Bill.h"
#include "BillStore.h"
#include "OrderManager.h"
#include "OrderStore.h"
#include "ProductCatalog.h"

class CheckoutService {
public:
    // productPath = full path of Product.txt (the other three paths live inside the stores).
    CheckoutService(ProductCatalog& catalog, OrderStore& orders, BillStore& bills, std::string productPath);

    // 1) check stock again  2) build the bill  3) update COPIES (stock down, order paid, bill added)
    // 4) write all four files with FileTransaction  5) only then update memory and clear the current order.
    // Any failure: no file and no stock number changes.
    OperationResult pay(OrderManager& manager, const PaymentInfo& payment, Bill& out);

    // For tests only: make the file writing fail on purpose.
    void simulateWriteFailure(bool enable) { simulateFailure_ = enable; }

private:
    ProductCatalog* catalog_;
    OrderStore* orders_;
    BillStore* bills_;
    std::string productPath_;
    bool simulateFailure_ = false;
};
