// OWNER: Member 4. STUB written by Member 5 so the project builds. Replace with the real code.
#include "CheckoutService.h"

CheckoutService::CheckoutService(ProductCatalog& catalog, OrderStore& orders, BillStore& bills, std::string productPath)
    : catalog_(&catalog), orders_(&orders), bills_(&bills), productPath_(std::move(productPath)) {}

OperationResult CheckoutService::pay(OrderManager&, const PaymentInfo&, Bill&) { return {false, "TODO (M4): pay"}; }
