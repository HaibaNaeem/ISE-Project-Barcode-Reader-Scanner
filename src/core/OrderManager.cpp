// OWNER: Member 3. STUB written by Member 5 so the project builds. Replace with the real code.
#include "OrderManager.h"

OrderManager::OrderManager(const ProductCatalog& catalog, OrderStore* store) : catalog_(&catalog), store_(store) {}

static OperationResult todo(const char* what) { return {false, std::string("TODO (M3): ") + what}; }

OperationResult OrderManager::startOrder(const std::string&, const std::string&) { return todo("startOrder"); }
OperationResult OrderManager::addByBarcode(const std::string&, Quantity) { return todo("addByBarcode"); }
OperationResult OrderManager::setQuantity(const std::string&, Quantity) { return todo("setQuantity"); }
OperationResult OrderManager::removeLine(const std::string&) { return todo("removeLine"); }
OperationResult OrderManager::hold() { return todo("hold"); }
OperationResult OrderManager::resume(const std::string&) { return todo("resume"); }
OperationResult OrderManager::cancel() { return todo("cancel"); }
std::vector<Order> OrderManager::heldOrders() const { return {}; }
