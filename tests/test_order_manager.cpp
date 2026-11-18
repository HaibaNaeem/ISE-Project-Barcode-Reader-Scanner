#include "../src/core/OrderManager.h"
#include "../src/core/ProductCatalog.h"
#include "../src/core/Product.h"
#include "../src/core/OrderStore.h"
#include "TestHelpers.h"
#include <iostream>

using Money = std::int64_t;
using Quantity = std::int64_t;

// Fake store to verify save is called after every change
class MockOrderStore : public OrderStore {
public:
    int saveCallCount = 0;
    std::vector<Order> savedOrders;

    OperationResult save(const Order& order) override {
        saveCallCount++;
        savedOrders.push_back(order);
        return OperationResult{true, "Mock save success", order.id, ""};
    }
};

int main() {
    // Create test products
    std::vector<Product> products = {
        {"P001", "100001", "Count Product", 10000, 5000, ProductType::Count},  // Stock 5
        {"P002", "100002", "Weight Product", 5000, 10000, ProductType::Weight}, // Stock 10
        {"P003", "100003", "Zero Stock Product", 2000, 0, ProductType::Count}   // Stock 0
    };

    ProductCatalog catalog(products);

    // Test with injectable date/time for determinism
    auto dateTimeProvider = []() -> std::string {
        return "2026-11-15 14:30:00";
    };

    OrderManager manager(catalog, nullptr, dateTimeProvider);

    // Test: startOrder creates new order
    auto r1 = manager.startOrder("John Doe", "555-1234");
    CHECK(r1.success, "startOrder succeeds");
    CHECK(manager.current().has_value(), "current order exists after startOrder");

    // Test: startOrder while another is active -> rejected
    auto r2 = manager.startOrder("Jane Doe", "555-5678");
    CHECK(!r2.success, "startOrder rejected while active order exists");

    // Test: add product with stock 5, add 2, add 3 merged into ONE line with qty 5
    auto r3 = manager.addByBarcode("100001", 2000);  // 2 pcs
    CHECK(r3.success, "add 2 pcs succeeds");
    CHECK(manager.current()->lines.size() == 1, "one line after first add");

    auto r4 = manager.addByBarcode("100001", 3000);  // 3 pcs
    CHECK(r4.success, "add 3 pcs succeeds");
    CHECK(manager.current()->lines.size() == 1, "still one line after second add (merged)");
    CHECK(manager.current()->lines[0].quantity == 5000, "merged quantity is 5");

    // Test: add 1 more rejected (stock 5, already have 5)
    auto r5 = manager.addByBarcode("100001", 1000);
    CHECK(!r5.success, "add 1 more rejected (stock exceeded)");

    // Test: unknown barcode rejected
    auto r6 = manager.addByBarcode("999999", 1000);
    CHECK(!r6.success, "unknown barcode rejected");

    // Test: quantity 0 rejected
    auto r7 = manager.addByBarcode("100001", 0);
    CHECK(!r7.success, "quantity 0 rejected");

    // Test: negative quantity rejected
    auto r8 = manager.addByBarcode("100001", -1000);
    CHECK(!r8.success, "negative quantity rejected");

    // Test: no active order rejected
    OrderManager manager2(catalog, nullptr, dateTimeProvider);
    auto r9 = manager2.addByBarcode("100001", 1000);
    CHECK(!r9.success, "add rejected when no active order");

    // Test: zero-stock product rejected
    auto r10 = manager.addByBarcode("100003", 1000);
    CHECK(!r10.success, "zero-stock product rejected");

    // Test: weight product 1.25 kg (1250) works
    auto r11 = manager.addByBarcode("100002", 1250);
    CHECK(r11.success, "weight product 1.25 kg works");

    // Test: count product with decimals (e.g., 1500) rejected
    auto r12 = manager.addByBarcode("100001", 1500);  // 1.5 pcs
    CHECK(!r12.success, "count product with decimals rejected");

    // Test: setQuantity works and re-checks stock
    auto r13 = manager.setQuantity("P001", 4000);  // Set to 4 pcs
    CHECK(r13.success, "setQuantity to 4 succeeds");
    CHECK(manager.current()->lines[0].quantity == 4000, "quantity updated to 4");

    // Test: setQuantity must not double count (check against NEW total)
    auto r14 = manager.setQuantity("P001", 5000);  // Set to 5 pcs
    CHECK(r14.success, "setQuantity to 5 succeeds");

    // Test: setQuantity with insufficient stock rejected
    auto r15 = manager.setQuantity("P001", 6000);  // Set to 6 pcs
    CHECK(!r15.success, "setQuantity with insufficient stock rejected");

    // Test: setQuantity for non-existent line rejected
    auto r16 = manager.setQuantity("P999", 1000);
    CHECK(!r16.success, "setQuantity for non-existent line rejected");

    // Test: removeLine works
    auto r17 = manager.removeLine("P002");
    CHECK(r17.success, "removeLine succeeds");
    CHECK(manager.current()->lines.size() == 1, "one line after removal");

    // Test: removeLine for non-existent line rejected
    auto r18 = manager.removeLine("P999");
    CHECK(!r18.success, "removeLine for non-existent line rejected");

    // Test: hold -> heldOrders has it, current empty
    auto r19 = manager.hold();
    CHECK(r19.success, "hold succeeds");
    CHECK(!manager.current().has_value(), "current empty after hold");
    CHECK(manager.heldOrders().size() == 1, "heldOrders has 1 order");

    std::string heldOrderId = manager.heldOrders()[0].id;

    // Test: resume works
    auto r20 = manager.resume(heldOrderId);
    CHECK(r20.success, "resume succeeds");
    CHECK(manager.current().has_value(), "current exists after resume");
    CHECK(manager.heldOrders().size() == 0, "heldOrders empty after resume");

    // Test: resume fails while another order is active
    OrderManager manager3(catalog, nullptr, dateTimeProvider);
    manager3.startOrder("Test", "123");
    auto r21 = manager3.resume(heldOrderId);
    CHECK(!r21.success, "resume rejected while active order exists");

    // Test: resume of a non-held id fails
    auto r22 = manager.resume("99999999");
    CHECK(!r22.success, "resume of non-held id rejected");

    // Test: cancel -> status archived, stock unchanged, current empty
    auto r23 = manager.cancel();
    CHECK(r23.success, "cancel succeeds");
    CHECK(!manager.current().has_value(), "current empty after cancel");

    // Test: fake store inside test verifies save is called after each change
    {
        MockOrderStore mockStore;
        OrderManager managerWithStore(catalog, &mockStore, dateTimeProvider);

        auto ms1 = managerWithStore.startOrder("Alice", "111-222");
        CHECK(ms1.success && mockStore.saveCallCount == 1, "store save called on startOrder");

        auto ms2 = managerWithStore.addByBarcode("100001", 1000);
        CHECK(ms2.success && mockStore.saveCallCount == 2, "store save called on addByBarcode");

        auto ms3 = managerWithStore.setQuantity("P001", 2000);
        CHECK(ms3.success && mockStore.saveCallCount == 3, "store save called on setQuantity");

        auto ms4 = managerWithStore.removeLine("P001");
        CHECK(ms4.success && mockStore.saveCallCount == 4, "store save called on removeLine");

        auto ms5 = managerWithStore.hold();
        CHECK(ms5.success && mockStore.saveCallCount == 5, "store save called on hold");

        std::string mockHeldId = managerWithStore.heldOrders()[0].id;
        auto ms6 = managerWithStore.resume(mockHeldId);
        CHECK(ms6.success && mockStore.saveCallCount == 6, "store save called on resume");

        auto ms7 = managerWithStore.cancel();
        CHECK(ms7.success && mockStore.saveCallCount == 7, "store save called on cancel");
    }

    std::cout << "\nPassed: " << passed << "  Failed: " << failed << "\n";
    return failed == 0 ? 0 : 1;
}
