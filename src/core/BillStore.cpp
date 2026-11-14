// OWNER: Member 4. STUB written by Member 5 so the project builds. Replace with the real code.
#include "BillStore.h"

BillStore::BillStore(std::string path) : path_(std::move(path)) {}
OperationResult BillStore::load() { return {false, "TODO (M4): BillStore::load"}; }
OperationResult BillStore::save() const { return {false, "TODO (M4): BillStore::save"}; }
std::string BillStore::serialize() const { return ""; }
std::string BillStore::nextBillId() const { return "000000000"; }
