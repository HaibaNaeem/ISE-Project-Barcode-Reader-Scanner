#include "ProductCatalog.h"
#include "OrderStore.h"
#include "BillStore.h"
#include "OrderManager.h"
#include "CheckoutService.h"
#include "Pricing.h"
#include "TestHelpers.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

int main() {
    std::string testDir = "test_int_run";
    fs::create_directories(testDir);

    std::string prodPath = testDir + "/Product.txt";
    std::string orderPath = testDir + "/Order.txt";
    std::string detailPath = testDir + "/OrderDetail.txt";
    std::string billPath = testDir + "/Bill.txt";

    // Copy sample Product.txt to our isolated test directory
    fs::copy_file("sample_data/Product.txt", prodPath, fs::copy_options::overwrite_existing);

    // Create empty files for order, detail, bill
    { std::ofstream(orderPath); }
    { std::ofstream(detailPath); }
    { std::ofstream(billPath); }

    // Initialize core objects
    ProductCatalog catalog;
    auto loadRes = catalog.load(prodPath);
    CHECK(loadRes.success);
    CHECK(catalog.all().size() >= 8);

    // Check initial stock of "123456" (Product 101, Milk Powder: stock 12 = 12000)
    auto p101 = catalog.findById("101");
    CHECK(p101.has_value());
    Quantity initialStock101 = p101->available;

    OrderStore store(orderPath, detailPath);
    CHECK(store.load().success);

    BillStore bills(billPath);
    CHECK(bills.load().success);

    OrderManager manager(catalog, &store);
    CheckoutService checkout(catalog, store, bills, prodPath);

    // 1. Start order
    auto startRes = manager.startOrder("Fatima Noor", "03001234567");
    CHECK(startRes.success);
    CHECK(manager.current().has_value());
    std::string currentOrderId = manager.current()->id;

    // 2. Scan three barcodes
    // Barcode 1: "123456" (Milk Powder, count, qty 2 = 2000)
    auto add1 = manager.addByBarcode("123456", 2000);
    CHECK(add1.success);

    // Barcode 2: "100230" (Rice 5kg, count, qty 1 = 1000)
    auto add2 = manager.addByBarcode("100230", 1000);
    CHECK(add2.success);

    // Barcode 3: "200145" (Sugar, weight, qty 1.25 kg = 1250)
    auto add3 = manager.addByBarcode("200145", 1250);
    CHECK(add3.success);

    CHECK_EQ(manager.current()->lines.size(), static_cast<size_t>(3));

    // 3. Price the bill draft
    Bill draftBill = buildBill(*manager.current(), catalog, "");
    CHECK_EQ(draftBill.lines.size(), static_cast<size_t>(3));
    CHECK(draftBill.payable > 0);

    // 4. Pay via CheckoutService (Cash payment)
    PaymentInfo payment;
    payment.mode = PaymentMode::Cash;
    payment.merchant = "";

    Bill outBill;
    auto payRes = checkout.pay(manager, payment, outBill);
    CHECK(payRes.success);
    CHECK(!manager.current().has_value());
    CHECK(!outBill.id.empty());
    CHECK(outBill.status == OrderStatus::Paid);

    // 5. Verify in-memory stock was reduced
    auto p101_after = catalog.findById("101");
    CHECK_EQ(p101_after->available, initialStock101 - 2000);

    // 6. Reload everything from disk into completely fresh instances to verify persistence
    ProductCatalog reloadedCatalog;
    CHECK(reloadedCatalog.load(prodPath).success);
    auto p101_reloaded = reloadedCatalog.findById("101");
    CHECK_EQ(p101_reloaded->available, initialStock101 - 2000);

    OrderStore reloadedStore(orderPath, detailPath);
    CHECK(reloadedStore.load().success);
    CHECK_EQ(reloadedStore.orders().size(), static_cast<size_t>(1));
    CHECK_EQ(reloadedStore.orders()[0].id, currentOrderId);
    CHECK(reloadedStore.orders()[0].status == OrderStatus::Paid);
    CHECK_EQ(reloadedStore.orders()[0].lines.size(), static_cast<size_t>(3));

    BillStore reloadedBills(billPath);
    CHECK(reloadedBills.load().success);
    CHECK_EQ(reloadedBills.bills().size(), static_cast<size_t>(1));
    CHECK_EQ(reloadedBills.bills()[0].id, outBill.id);

    // Clean up test directory
    fs::remove_all(testDir);

    return testSummary("test_integration");
}
