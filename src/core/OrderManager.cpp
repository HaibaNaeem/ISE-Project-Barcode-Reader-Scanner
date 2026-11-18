// OWNER: Member 3. Shared contract (frozen). The order being recorded + held orders.
#include "OrderManager.h"
#include "StockRules.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>

OrderManager::OrderManager(const ProductCatalog& catalog, OrderStore* store,
                           std::function<std::string()> dateTimeProvider)
    : catalog_(&catalog), store_(store), legacyClock_(std::move(dateTimeProvider)) {}

DateTime OrderManager::getNow() const {
    if (clock_) {
        return clock_();
    }
    if (legacyClock_) {
        std::string s = legacyClock_();
        DateTime dt;
        if (s.size() >= 10) {
            dt.date = s.substr(0, 10);
            try {
                dt.dayOfMonth = std::stoi(dt.date.substr(8, 2));
            } catch (...) {
                dt.dayOfMonth = 1;
            }
        } else {
            dt.date = "2026-11-16";
            dt.dayOfMonth = 16;
        }
        if (s.size() >= 19) {
            dt.time = s.substr(11, 8);
        } else {
            dt.time = "14:05:09";
        }
        return dt;
    }

    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);

    std::tm tm_buf{};
#if defined(_WIN32)
    localtime_s(&tm_buf, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_buf);
#endif

    std::ostringstream ssDate, ssTime;
    ssDate << std::put_time(&tm_buf, "%Y-%m-%d");
    ssTime << std::put_time(&tm_buf, "%H:%M:%S");

    DateTime dt;
    dt.date = ssDate.str();
    dt.time = ssTime.str();
    dt.dayOfMonth = tm_buf.tm_mday;
    return dt;
}

void OrderManager::syncCurrentToStore() {
    if (!current_.has_value()) return;

    if (store_) {
        store_->save(*current_);
    } else {
        bool found = false;
        for (auto& ord : inMemoryOrders_) {
            if (ord.id == current_->id) {
                ord = *current_;
                found = true;
                break;
            }
        }
        if (!found) {
            inMemoryOrders_.push_back(*current_);
        }
    }
}

OperationResult OrderManager::startOrder(const std::string& customerName, const std::string& customerContact) {
    if (current_.has_value()) {
        return {false, "An active order is already in progress. Please hold or cancel the current order first."};
    }

    DateTime now = getNow();
    std::string orderId;
    if (store_) {
        orderId = store_->nextOrderId(now.dayOfMonth);
    } else {
        orderId = OrderStore::generateNextOrderId(inMemoryOrders_, now.dayOfMonth);
    }

    Order newOrder;
    newOrder.id = orderId;
    newOrder.date = now.date;
    newOrder.time = now.time;
    newOrder.customerName = customerName;
    newOrder.customerContact = customerContact;
    newOrder.status = OrderStatus::InProgress;
    newOrder.paymentAmount = 0;
    newOrder.mode = PaymentMode::None;
    newOrder.merchant = "";
    newOrder.lines.clear();

    current_ = newOrder;
    syncCurrentToStore();

    return {true, "Order started successfully. ID: " + newOrder.id, newOrder.id};
}

OperationResult OrderManager::addByBarcode(const std::string& barcode, Quantity quantity) {
    if (!current_.has_value()) {
        return {false, "No active order. Please start an order first."};
    }
    if (quantity <= 0) {
        return {false, "Quantity must be greater than zero."};
    }

    auto prodOpt = catalog_->findByBarcode(barcode);
    if (!prodOpt.has_value()) {
        return {false, "Product not found with barcode: " + barcode};
    }

    const Product& product = *prodOpt;

    if (product.available <= 0) {
        return {false, "Product is out of stock: " + product.name};
    }

    if (product.unit == MeasureUnit::Count && (quantity % 1000 != 0)) {
        return {false, "Count products require whole quantities (e.g., 1, 2, 3)."};
    }

    Quantity alreadyInCart = 0;
    for (const auto& line : current_->lines) {
        if (line.productId == product.id) {
            alreadyInCart = line.quantity;
            break;
        }
    }

    if (!StockRules::hasEnoughStock(alreadyInCart, quantity, product.available)) {
        return {false, "Not enough stock for product '" + product.name + "'."};
    }

    bool found = false;
    for (auto& line : current_->lines) {
        if (line.productId == product.id) {
            line.quantity += quantity;
            found = true;
            break;
        }
    }

    if (!found) {
        current_->lines.push_back(OrderLine{product.id, product.barcode, quantity});
    }

    syncCurrentToStore();
    return {true, "Product added to order: " + product.name, product.id};
}

OperationResult OrderManager::setQuantity(const std::string& productId, Quantity quantity) {
    if (!current_.has_value()) {
        return {false, "No active order. Please start an order first."};
    }
    if (quantity <= 0) {
        return {false, "Quantity must be greater than zero."};
    }

    auto prodOpt = catalog_->findById(productId);
    if (!prodOpt.has_value()) {
        return {false, "Product not found in catalog: " + productId};
    }
    const Product& product = *prodOpt;

    if (product.unit == MeasureUnit::Count && (quantity % 1000 != 0)) {
        return {false, "Count products require whole quantities (e.g., 1, 2, 3)."};
    }

    // When setting quantity, new total is tested against available stock
    if (!StockRules::hasEnoughStock(0, quantity, product.available)) {
        return {false, "Not enough stock for product '" + product.name + "'."};
    }

    bool found = false;
    for (auto& line : current_->lines) {
        if (line.productId == productId) {
            line.quantity = quantity;
            found = true;
            break;
        }
    }

    if (!found) {
        return {false, "Product not found in current order: " + productId};
    }

    syncCurrentToStore();
    return {true, "Quantity updated successfully.", productId};
}

OperationResult OrderManager::removeLine(const std::string& productId) {
    if (!current_.has_value()) {
        return {false, "No active order. Please start an order first."};
    }

    auto it = std::find_if(current_->lines.begin(), current_->lines.end(),
                           [&productId](const OrderLine& line) { return line.productId == productId; });
    if (it == current_->lines.end()) {
        return {false, "Product not found in current order: " + productId};
    }

    current_->lines.erase(it);
    syncCurrentToStore();
    return {true, "Product removed from order.", productId};
}

OperationResult OrderManager::hold() {
    if (!current_.has_value()) {
        return {false, "No active order to hold."};
    }

    current_->status = OrderStatus::OnHold;
    std::string orderId = current_->id;
    syncCurrentToStore();
    current_.reset();

    return {true, "Order placed on hold. ID: " + orderId, orderId};
}

OperationResult OrderManager::resume(const std::string& orderId) {
    if (current_.has_value()) {
        return {false, "An active order is already in progress. Please hold or cancel the current order first."};
    }

    if (store_) {
        auto& all = store_->orders();
        auto it = std::find_if(all.begin(), all.end(),
                               [&orderId](const Order& ord) {
                                   return ord.id == orderId && ord.status == OrderStatus::OnHold;
                               });
        if (it == all.end()) {
            return {false, "No on-hold order found with ID: " + orderId};
        }
        it->status = OrderStatus::InProgress;
        current_ = *it;
        syncCurrentToStore();
        return {true, "Order resumed successfully. ID: " + orderId, orderId};
    } else {
        auto it = std::find_if(inMemoryOrders_.begin(), inMemoryOrders_.end(),
                               [&orderId](const Order& ord) {
                                   return ord.id == orderId && ord.status == OrderStatus::OnHold;
                               });
        if (it == inMemoryOrders_.end()) {
            return {false, "No on-hold order found with ID: " + orderId};
        }
        it->status = OrderStatus::InProgress;
        current_ = *it;
        return {true, "Order resumed successfully. ID: " + orderId, orderId};
    }
}

OperationResult OrderManager::cancel() {
    if (!current_.has_value()) {
        return {false, "No active order to cancel."};
    }

    current_->status = OrderStatus::Archived;
    std::string orderId = current_->id;
    syncCurrentToStore();
    current_.reset();

    return {true, "Order cancelled and archived. ID: " + orderId, orderId};
}

std::vector<Order> OrderManager::heldOrders() const {
    std::vector<Order> held;
    if (store_) {
        for (const auto& ord : store_->orders()) {
            if (ord.status == OrderStatus::OnHold) {
                held.push_back(ord);
            }
        }
    } else {
        for (const auto& ord : inMemoryOrders_) {
            if (ord.status == OrderStatus::OnHold) {
                held.push_back(ord);
            }
        }
    }
    return held;
}
