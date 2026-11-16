#pragma once
// OWNER: Member 4.
// Orchestrates atomic all-or-nothing checkout: validates payment, verifies stock,
// builds the bill, writes all 4 files via FileTransaction, and updates in-memory state only upon commit.
#include <string>
#include "CommonTypes.h"
#include "Bill.h"
#include "BillStore.h"
#include "OrderManager.h"
#include "OrderStore.h"
#include "ProductCatalog.h"

class CheckoutService {
public:
    // Injected dependencies: catalog, order store, bill store, and path to Product.txt.
    CheckoutService(ProductCatalog& catalog, OrderStore& orders, BillStore& bills, std::string productPath);

    // 1) Validate current order & payment details.
    // 2) Check stock again using StockRules::hasEnoughStock.
    // 3) Build final Bill.
    // 4) Update COPIES (stock reduced, order marked Paid, bill added).
    // 5) Commit all four files with FileTransaction.
    // 6) Only upon success: update memory, clear current order in manager, and set out bill.
    OperationResult pay(OrderManager& manager, const PaymentInfo& payment, Bill& out);

    // For testing: force FileTransaction failure to verify all-or-nothing rollback.
    void simulateWriteFailure(bool enable) { simulateFailure_ = enable; }

private:
    ProductCatalog* catalog_;
    OrderStore* orders_;
    BillStore* bills_;
    std::string productPath_;
    bool simulateFailure_ = false;
};
