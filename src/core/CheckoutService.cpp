// OWNER: Member 4.
// Atomic payment orchestration implementation.
#include "CheckoutService.h"
#include "Pricing.h"
#include "StockRules.h"
#include "RecordFormat.h"
#include "Units.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace {

// Format money string helper
std::string moneyToStr(Money amount) {
    std::string s = formatMoney(amount);
    if (s != "TODO") return s;
    std::string sign = amount < 0 ? "-" : "";
    if (amount < 0) amount = -amount;
    std::string cents = std::to_string(amount % 100);
    if (cents.size() < 2) cents = "0" + cents;
    return sign + std::to_string(amount / 100) + "." + cents;
}

// Format quantity string helper
std::string qtyToStr(Quantity qty) {
    if (qty % 1000 == 0) return std::to_string(qty / 1000);
    std::string q = std::to_string(qty % 1000);
    while (q.size() < 3) q = "0" + q;
    return std::to_string(qty / 1000) + "." + q;
}

// Fallback serializer for ProductCatalog if M2 serialize() on dev is still returning ""
std::string serializeProductCatalogFallback(const ProductCatalog& cat) {
    std::string out = cat.serialize();
    if (!out.empty()) {
        return out;
    }
    std::string text;
    for (const auto& p : cat.all()) {
        std::string unitStr = (p.unit == MeasureUnit::Weight) ? "weight" : "count";
        std::vector<std::string> fields = {
            p.id,
            p.barcode,
            p.acronym,
            quoteField(p.name),
            unitStr,
            moneyToStr(p.unitPrice),
            moneyToStr(p.taxPerUnit),
            moneyToStr(p.discountPerUnit),
            qtyToStr(p.available),
            quoteField(p.barcodeImagePath)
        };
        std::string rec;
        for (size_t i = 0; i < fields.size(); ++i) {
            if (i > 0) rec += " ";
            rec += fields[i];
        }
        rec += " #\n";
        text += rec;
    }
    return text;
}

// Fallback serializer for OrderStore Orders if M3 serializeOrders() on dev is still returning ""
std::string serializeOrdersFallback(const OrderStore& store) {
    std::string out = store.serializeOrders();
    if (!out.empty()) {
        return out;
    }
    std::string text;
    for (const auto& ord : store.orders()) {
        std::vector<std::string> fields = {
            ord.id,
            ord.date.empty() ? "2026-11-16" : ord.date,
            ord.time.empty() ? "00:00:00" : ord.time,
            quoteField(ord.customerName),
            quoteField(ord.customerContact),
            orderStatusToText(ord.status),
            moneyToStr(ord.paymentAmount),
            paymentModeToText(ord.mode),
            quoteField(ord.merchant)
        };
        std::string rec;
        for (size_t i = 0; i < fields.size(); ++i) {
            if (i > 0) rec += " ";
            rec += fields[i];
        }
        rec += " #\n";
        text += rec;
    }
    return text;
}

// Fallback serializer for OrderStore Details if M3 serializeDetails() on dev is still returning ""
std::string serializeDetailsFallback(const OrderStore& store) {
    std::string out = store.serializeDetails();
    if (!out.empty()) {
        return out;
    }
    std::string text;
    for (const auto& ord : store.orders()) {
        for (const auto& line : ord.lines) {
            std::vector<std::string> fields = {
                ord.id,
                line.productId,
                qtyToStr(line.quantity)
            };
            std::string rec;
            for (size_t i = 0; i < fields.size(); ++i) {
                if (i > 0) rec += " ";
                rec += fields[i];
            }
            rec += " #\n";
            text += rec;
        }
    }
    return text;
}

// Executes an atomic commit of all 4 files with full rollback capability
OperationResult commitAllOrNothing(
    const std::vector<std::pair<std::string, std::string>>& fileEntries,
    bool simulateFailure)
{
    // Try calling M2's FileTransaction first
    FileTransaction ft;
    ft.simulateFailure(simulateFailure);
    for (const auto& [target, content] : fileEntries) {
        if (!target.empty()) {
            ft.add(target, content);
        }
    }
    OperationResult ftRes = ft.commit();
    if (ftRes.message.find("TODO") == std::string::npos) {
        // M2's real FileTransaction ran
        return ftRes;
    }

    // Fallback transaction if M2's commit() is still a stub in dev branch
    std::error_code ec;
    auto cleanupTmp = [&] {
        for (const auto& [target, _] : fileEntries) {
            if (!target.empty()) fs::remove(target + ".tmp", ec);
        }
    };

    // Step 1: Write all content to temporary files
    for (const auto& [target, content] : fileEntries) {
        if (target.empty()) continue;
        std::ofstream out(target + ".tmp", std::ios::binary);
        if (!out.is_open()) {
            cleanupTmp();
            return {false, "Failed to create temp file: " + target + ".tmp"};
        }
        out << content;
        out.close();
        if (!out) {
            cleanupTmp();
            return {false, "Failed to write temp file: " + target + ".tmp"};
        }
    }

    // Simulated failure hook for testing all-or-nothing rollback
    if (simulateFailure) {
        cleanupTmp();
        return {false, "Transaction failed (simulated). No file was modified."};
    }

    // Step 2: Swap files atomically via backup rename
    size_t replacedCount = 0;
    bool success = true;
    for (size_t i = 0; i < fileEntries.size(); ++i) {
        const std::string& target = fileEntries[i].first;
        if (target.empty()) continue;

        if (fs::exists(target)) {
            fs::rename(target, target + ".bak", ec);
            if (ec) {
                success = false;
                break;
            }
        }

        fs::rename(target + ".tmp", target, ec);
        if (ec) {
            success = false;
            break;
        }
        ++replacedCount;
    }

    if (!success) {
        // Rollback any replaced files
        for (size_t i = 0; i <= replacedCount && i < fileEntries.size(); ++i) {
            const std::string& target = fileEntries[i].first;
            if (target.empty()) continue;
            if (fs::exists(target + ".bak")) {
                fs::remove(target, ec);
                fs::rename(target + ".bak", target, ec);
            }
        }
        cleanupTmp();
        return {false, "Transaction failed during file replacement. Rollback completed."};
    }

    // Clean up backup files on success
    for (const auto& [target, _] : fileEntries) {
        if (!target.empty()) fs::remove(target + ".bak", ec);
    }

    return {true, "Files committed successfully."};
}

} // anonymous namespace

CheckoutService::CheckoutService(
    ProductCatalog& catalog,
    OrderStore& orders,
    BillStore& bills,
    std::string productPath)
    : catalog_(&catalog), orders_(&orders), bills_(&bills), productPath_(std::move(productPath)) {}

OperationResult CheckoutService::pay(OrderManager& manager, const PaymentInfo& payment, Bill& out) {
    // -------------------------------------------------------------
    // Step 1: Validate active order and payment details
    // -------------------------------------------------------------
    if (!manager.current().has_value()) {
        return {false, "No active order to checkout."};
    }

    const Order& currentOrder = *manager.current();
    if (currentOrder.lines.empty()) {
        return {false, "Cannot checkout an empty order."};
    }

    if (payment.mode == PaymentMode::None) {
        return {false, "Please select a valid payment mode (Cash, Card, or Wallet)."};
    }

    if (payment.mode == PaymentMode::Card || payment.mode == PaymentMode::Wallet) {
        if (payment.merchant.empty()) {
            return {false, "Merchant name is required for Card and Wallet payments."};
        }
    } else if (payment.mode == PaymentMode::Cash) {
        if (!payment.merchant.empty()) {
            return {false, "Merchant name must be empty for Cash payments."};
        }
    }

    // -------------------------------------------------------------
    // Step 2: Check stock AGAIN for every order line using StockRules
    // -------------------------------------------------------------
    for (const auto& line : currentOrder.lines) {
        std::optional<Product> prod = catalog_->findById(line.productId);
        if (!prod && !line.barcode.empty()) {
            prod = catalog_->findByBarcode(line.barcode);
        }

        if (!prod) {
            return {false, "Product not found in catalog: " + line.productId};
        }

        // Verify sufficient stock remains (cannot sell more than available)
        if (!hasEnoughStock(0, line.quantity, prod->available)) {
            return {false, "Insufficient stock for product '" + prod->name + "'."};
        }
    }

    // -------------------------------------------------------------
    // Step 3: Build the final bill draft
    // -------------------------------------------------------------
    std::string nextId = bills_->nextBillId();
    Bill finalBill = buildBill(currentOrder, *catalog_, nextId);
    if (finalBill.lines.empty()) {
        return {false, "Failed to price order lines."};
    }

    // Validate payment amount equals bill total
    if (currentOrder.paymentAmount != 0 && currentOrder.paymentAmount != finalBill.payable) {
        return {false, "Payment amount does not equal the bill total."};
    }

    // -------------------------------------------------------------
    // Step 4: Work on COPIES ONLY (All-or-Nothing preparation)
    // -------------------------------------------------------------
    // Copy of Catalog with stock reduced
    ProductCatalog catalogCopy = *catalog_;
    for (const auto& line : currentOrder.lines) {
        OperationResult reduceRes = catalogCopy.reduceStock(line.productId, line.quantity);
        if (!reduceRes.success) {
            return {false, "Failed to deduct stock for product ID " + line.productId};
        }
    }

    // Copy of Order updated to Paid status
    Order paidOrder = currentOrder;
    paidOrder.status = OrderStatus::Paid;
    paidOrder.mode = payment.mode;
    paidOrder.merchant = payment.merchant;
    paidOrder.paymentAmount = finalBill.payable;

    // Finalize bill metadata
    finalBill.status = OrderStatus::Paid;
    finalBill.paymentMode = payment.mode;
    finalBill.merchant = payment.merchant;
    finalBill.date = paidOrder.date;
    finalBill.time = paidOrder.time;

    // Copy of BillStore with new bill added
    BillStore billsCopy = *bills_;
    billsCopy.addBill(finalBill);

    // Copy of OrderStore with paid order updated
    OrderStore ordersCopy = *orders_;
    bool orderUpdated = false;
    for (auto& ord : ordersCopy.orders()) {
        if (ord.id == paidOrder.id) {
            ord = paidOrder;
            orderUpdated = true;
            break;
        }
    }
    if (!orderUpdated) {
        ordersCopy.orders().push_back(paidOrder);
    }

    // -------------------------------------------------------------
    // Step 5: Write all four files atomically via FileTransaction
    // -------------------------------------------------------------
    std::string productContent = serializeProductCatalogFallback(catalogCopy);
    std::string orderContent   = serializeOrdersFallback(ordersCopy);
    std::string detailContent  = serializeDetailsFallback(ordersCopy);
    std::string billContent    = billsCopy.serialize();

    std::vector<std::pair<std::string, std::string>> filesToCommit;
    if (!productPath_.empty()) {
        filesToCommit.push_back({productPath_, productContent});
    }
    if (!orders_->orderPath().empty()) {
        filesToCommit.push_back({orders_->orderPath(), orderContent});
    }
    if (!orders_->detailPath().empty()) {
        filesToCommit.push_back({orders_->detailPath(), detailContent});
    }
    if (!bills_->path().empty()) {
        filesToCommit.push_back({bills_->path(), billContent});
    }

    OperationResult commitRes = commitAllOrNothing(filesToCommit, simulateFailure_);
    if (!commitRes.success) {
        // Rollback: files, catalog, orders, and manager remain completely unchanged
        return {false, "Checkout failed: " + commitRes.message};
    }

    // -------------------------------------------------------------
    // Step 6: ONLY after transaction succeeds, update in-memory state
    // -------------------------------------------------------------
    *catalog_ = std::move(catalogCopy);
    *orders_  = std::move(ordersCopy);
    *bills_   = std::move(billsCopy);

    // Clear active order in manager
    manager.clearCurrent();

    // Populate output bill
    out = finalBill;

    return {true, "Payment successful.", finalBill.id};
}
