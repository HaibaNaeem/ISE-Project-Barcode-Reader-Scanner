#include "OrderManager.h"
#include "StockRules.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace {

// Integer-only quantity formatting (no float/double in src/core)
// Quantity is in thousandths (e.g. 5000 -> "5", 1250 -> "1.25", 1000 -> "1")
std::string formatQuantity(Quantity q) {
    int64_t whole = q / 1000;
    int64_t frac = q % 1000;
    if (frac < 0) frac = -frac;
    if (frac == 0) {
        return std::to_string(whole);
    }
    std::string fracStr = std::to_string(frac);
    while (fracStr.length() < 3) {
        fracStr = "0" + fracStr;
    }
    while (!fracStr.empty() && fracStr.back() == '0') {
        fracStr.pop_back();
    }
    return std::to_string(whole) + "." + fracStr;
}

} // namespace

OrderManager::OrderManager(const ProductCatalog& catalog,
                           OrderStore* store,
                           std::function<std::string()> dateTimeProvider)
    : catalog_(catalog)
    , store_(store)
    , dateTimeProvider_(dateTimeProvider) {}

std::string OrderManager::getCurrentDateTime() const {
    if (dateTimeProvider_) {
        return dateTimeProvider_();
    }

    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);

    std::tm tm_buf{};
#if defined(_WIN32)
    localtime_s(&tm_buf, &time_t);
#else
    localtime_r(&time_t, &tm_buf);
#endif

    std::stringstream ss;
    ss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string OrderManager::generateOrderId(int dayOfMonth) const {
    // Delegate to OrderStore's pure function for ID generation
    return OrderStore::generateNextOrderId(allOrders_, dayOfMonth);
}

OperationResult OrderManager::startOrder(const std::string& name, const std::string& contact) {
    if (currentOrder_.has_value()) {
        return OperationResult{false, "An active order is already in progress. Please hold or cancel the current order first.", "", ""};
    }

    std::string dateTime = getCurrentDateTime();
    std::string date = dateTime.substr(0, 10);  // YYYY-MM-DD
    std::string time = dateTime.substr(11, 8);  // HH:MM:SS

    // Extract day of month from date
    int dayOfMonth = std::stoi(date.substr(8, 2));

    Order newOrder;
    newOrder.id = generateOrderId(dayOfMonth);
    newOrder.date = date;
    newOrder.time = time;
    newOrder.customerName = name;
    newOrder.customerContact = contact;
    newOrder.status = OrderStatus::InProgress;
    newOrder.paymentAmount = 0;
    newOrder.mode = PaymentMode::None;
    newOrder.merchant = "";
    newOrder.lines.clear();

    currentOrder_ = newOrder;
    allOrders_.push_back(newOrder);

    // Save to store if available
    if (store_) {
        store_->save(newOrder);
    }

    return OperationResult{true, "Order started successfully. ID: " + newOrder.id, newOrder.id, ""};
}

OperationResult OrderManager::addByBarcode(const std::string& barcode, Quantity qty) {
    if (!currentOrder_.has_value()) {
        return OperationResult{false, "No active order. Please start an order first.", "", ""};
    }

    if (qty <= 0) {
        return OperationResult{false, "Quantity must be greater than zero.", "", ""};
    }

    auto productOpt = catalog_.findByBarcode(barcode);
    if (!productOpt.has_value()) {
        return OperationResult{false, "Product not found with barcode: " + barcode, "", ""};
    }

    const Product& product = productOpt.value();

    // Check stock availability
    if (product.available <= 0) {
        return OperationResult{false, "Product is out of stock: " + product.name, "", ""};
    }

    // For count products, quantity must be a whole number (multiple of 1000)
    if (product.type == ProductType::Count && (qty % 1000 != 0)) {
        return OperationResult{false, "Count products require whole quantities (e.g., 1, 2, 3).", "", ""};
    }

    // Calculate quantity already in cart for this product
    Quantity alreadyInCart = 0;
    for (const auto& line : currentOrder_->lines) {
        if (line.productId == product.id) {
            alreadyInCart = line.quantity;
            break;
        }
    }

    // Check stock rule
    if (!StockRules::hasEnoughStock(alreadyInCart, qty, product.available)) {
        return OperationResult{false, "Not enough stock. Available: " + formatQuantity(product.available) + ", requested: " + formatQuantity(qty), "", ""};
    }

    // Add or merge line
    bool found = false;
    for (auto& line : currentOrder_->lines) {
        if (line.productId == product.id) {
            line.quantity += qty;
            found = true;
            break;
        }
    }

    if (!found) {
        OrderLine newLine;
        newLine.productId = product.id;
        newLine.barcode = product.barcode;
        newLine.quantity = qty;
        currentOrder_->lines.push_back(newLine);
    }

    // Update in allOrders_
    for (auto& order : allOrders_) {
        if (order.id == currentOrder_->id) {
            order = currentOrder_.value();
            break;
        }
    }

    // Save to store if available
    if (store_) {
        store_->save(currentOrder_.value());
    }

    return OperationResult{true, "Product added to order.", product.id, ""};
}

OperationResult OrderManager::setQuantity(const std::string& productId, Quantity qty) {
    if (!currentOrder_.has_value()) {
        return OperationResult{false, "No active order. Please start an order first.", "", ""};
    }

    if (qty <= 0) {
        return OperationResult{false, "Quantity must be greater than zero.", "", ""};
    }

    auto productOpt = catalog_.findById(productId);
    if (!productOpt.has_value()) {
        return OperationResult{false, "Product not found with ID: " + productId, "", ""};
    }

    const Product& product = productOpt.value();

    // Check stock availability
    if (product.available <= 0) {
        return OperationResult{false, "Product is out of stock: " + product.name, "", ""};
    }

    // For count products, quantity must be a whole number (multiple of 1000)
    if (product.type == ProductType::Count && (qty % 1000 != 0)) {
        return OperationResult{false, "Count products require whole quantities (e.g., 1, 2, 3).", "", ""};
    }

    // Find the line
    bool found = false;
    for (auto& line : currentOrder_->lines) {
        if (line.productId == productId) {
            found = true;

            // Check stock rule against NEW total (not added on top)
            if (!StockRules::hasEnoughStock(0, qty, product.available)) {
                return OperationResult{false, "Not enough stock. Available: " + formatQuantity(product.available) + ", requested: " + formatQuantity(qty), "", ""};
            }

            line.quantity = qty;
            break;
        }
    }

    if (!found) {
        return OperationResult{false, "Product not found in current order: " + productId, "", ""};
    }

    // Update in allOrders_
    for (auto& order : allOrders_) {
        if (order.id == currentOrder_->id) {
            order = currentOrder_.value();
            break;
        }
    }

    // Save to store if available
    if (store_) {
        store_->save(currentOrder_.value());
    }

    return OperationResult{true, "Quantity updated successfully.", productId, ""};
}

OperationResult OrderManager::removeLine(const std::string& productId) {
    if (!currentOrder_.has_value()) {
        return OperationResult{false, "No active order. Please start an order first.", "", ""};
    }

    auto it = std::find_if(currentOrder_->lines.begin(), currentOrder_->lines.end(),
                           [&productId](const OrderLine& line) { return line.productId == productId; });

    if (it == currentOrder_->lines.end()) {
        return OperationResult{false, "Product not found in current order: " + productId, "", ""};
    }

    currentOrder_->lines.erase(it);

    // Update in allOrders_
    for (auto& order : allOrders_) {
        if (order.id == currentOrder_->id) {
            order = currentOrder_.value();
            break;
        }
    }

    // Save to store if available
    if (store_) {
        store_->save(currentOrder_.value());
    }

    return OperationResult{true, "Product removed from order.", productId, ""};
}

OperationResult OrderManager::hold() {
    if (!currentOrder_.has_value()) {
        return OperationResult{false, "No active order to hold.", "", ""};
    }

    currentOrder_->status = OrderStatus::OnHold;

    // Update in allOrders_
    for (auto& order : allOrders_) {
        if (order.id == currentOrder_->id) {
            order = currentOrder_.value();
            break;
        }
    }

    // Save to store if available
    if (store_) {
        store_->save(currentOrder_.value());
    }

    std::string orderId = currentOrder_->id;
    currentOrder_ = std::nullopt;

    return OperationResult{true, "Order placed on hold. ID: " + orderId, orderId, ""};
}

OperationResult OrderManager::resume(const std::string& orderId) {
    if (currentOrder_.has_value()) {
        return OperationResult{false, "An active order is already in progress. Please hold or cancel the current order first.", "", ""};
    }

    // Find the on-hold order
    auto it = std::find_if(allOrders_.begin(), allOrders_.end(),
                           [&orderId](const Order& order) {
                               return order.id == orderId && order.status == OrderStatus::OnHold;
                           });

    if (it == allOrders_.end()) {
        return OperationResult{false, "No on-hold order found with ID: " + orderId, "", ""};
    }

    it->status = OrderStatus::InProgress;
    currentOrder_ = *it;

    // Save to store if available
    if (store_) {
        store_->save(currentOrder_.value());
    }

    return OperationResult{true, "Order resumed successfully. ID: " + orderId, orderId, ""};
}

OperationResult OrderManager::cancel() {
    if (!currentOrder_.has_value()) {
        return OperationResult{false, "No active order to cancel.", "", ""};
    }

    currentOrder_->status = OrderStatus::Archived;

    // Update in allOrders_
    for (auto& order : allOrders_) {
        if (order.id == currentOrder_->id) {
            order = currentOrder_.value();
            break;
        }
    }

    // Save to store if available
    if (store_) {
        store_->save(currentOrder_.value());
    }

    std::string orderId = currentOrder_->id;
    currentOrder_ = std::nullopt;

    return OperationResult{true, "Order cancelled and archived. ID: " + orderId, orderId, ""};
}

const std::optional<Order>& OrderManager::current() const {
    return currentOrder_;
}

std::vector<Order> OrderManager::heldOrders() const {
    std::vector<Order> held;
    for (const auto& order : allOrders_) {
        if (order.status == OrderStatus::OnHold) {
            held.push_back(order);
        }
    }
    return held;
}
