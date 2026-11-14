#pragma once
// OWNER: Member 2. Shared contract (frozen). Product.txt.
#include <optional>
#include <string>
#include <vector>
#include "CommonTypes.h"
#include "Product.h"

class ProductCatalog {
public:
    ProductCatalog() = default;
    explicit ProductCatalog(std::vector<Product> products);   // in memory - use this in tests

    OperationResult load(const std::string& path);            // bad rows skipped; warnings go in message
    OperationResult save(const std::string& path) const;
    std::string serialize() const;                            // whole Product.txt as text (used by CheckoutService)

    std::optional<Product> findByBarcode(const std::string& barcode) const;
    std::optional<Product> findById(const std::string& id) const;

    const std::vector<Product>& all() const { return products_; }
    const std::vector<std::string>& warnings() const { return warnings_; }   // skipped rows etc.

    // Used by CheckoutService at payment time. Fails if stock would go below zero.
    OperationResult reduceStock(const std::string& productId, Quantity amount);

private:
    std::vector<Product> products_;
    std::vector<std::string> warnings_;
};
