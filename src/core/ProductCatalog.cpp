// OWNER: Member 2. Member 5 wrote the in-memory lookups so M3 can start on day one.
// load / save / serialize / validation are still TODO (M2).
#include "ProductCatalog.h"

ProductCatalog::ProductCatalog(std::vector<Product> products) : products_(std::move(products)) {}

OperationResult ProductCatalog::load(const std::string&) { return {false, "TODO (M2): ProductCatalog::load"}; }
OperationResult ProductCatalog::save(const std::string&) const { return {false, "TODO (M2): ProductCatalog::save"}; }
std::string ProductCatalog::serialize() const { return ""; }

std::optional<Product> ProductCatalog::findByBarcode(const std::string& barcode) const {
    for (const auto& p : products_) if (p.barcode == barcode) return p;
    return std::nullopt;
}
std::optional<Product> ProductCatalog::findById(const std::string& id) const {
    for (const auto& p : products_) if (p.id == id) return p;
    return std::nullopt;
}
OperationResult ProductCatalog::reduceStock(const std::string& productId, Quantity amount) {
    for (auto& p : products_) {
        if (p.id != productId) continue;
        if (amount < 0 || p.available < amount) return {false, "Not enough stock"};
        p.available -= amount;
        return {true, "Stock updated"};
    }
    return {false, "Unknown product"};
}
