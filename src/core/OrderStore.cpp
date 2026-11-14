// OWNER: Member 3. STUB written by Member 5 so the project builds. Replace with the real code.
#include "OrderStore.h"

OrderStore::OrderStore(std::string orderPath, std::string detailPath)
    : orderPath_(std::move(orderPath)), detailPath_(std::move(detailPath)) {}

OperationResult OrderStore::load() { return {false, "TODO (M3): OrderStore::load"}; }
OperationResult OrderStore::save() const { return {false, "TODO (M3): OrderStore::save"}; }
std::string OrderStore::serializeOrders() const { return ""; }
std::string OrderStore::serializeDetails() const { return ""; }
std::string OrderStore::nextOrderId(int) const { return "00000000"; }
