#include "OrderManager.h"
#include "ProductCatalog.h"
#include "Product.h"
#include "OrderStore.h"
#include "TestHelpers.h"

// Fake store to verify save is called after every change
class MockOrderStore : public OrderStore {
public:
    int saveCallCount = 0;
    std::vector<Order> savedOrders;

    OperationResult save(const Order& order) override {
        saveCallCount++;
        savedOrders.push_back(order);
        bool found = false;
        for (auto& ord : orders()) {
            if (ord.id == order.id) {
                ord = order;
                found = true;
                break;
            }
        }
        if (!found) {
            orders().push_back(order);
        }
        return OperationResult{true, "Mock save success", order.id, ""};
    }
};

int main() {
    // Create test products
    std::vector<Product> products = {
        {"P001", "100001", "P001", "Count Product", MeasureUnit::Count, 10000, 0, 0, 5000, ""},   // Stock 5
        {"P002", "100002", "P002", "Weight Product", MeasureUnit::Weight, 5000, 0, 0, 10000, ""}, // Stock 10
        {"P003", "100003", "P003", "Zero Stock Product", MeasureUnit::Count, 2000, 0, 0, 0, ""}    // Stock 0
    };

    ProductCatalog catalog(products);

    // Test with injectable date/time for determinism
    auto dateTimeProvider = []() -> std::string {
        return "2026-11-15 14:30:00";
    };

    OrderManager manager(catalog, nullptr, dateTimeProvider);

    // Test: startOrder creates new order
    auto r1 = manager.startOrder("John Doe", "555-1234");
    CHECK(r1.success);
    CHECK(manager.current().has_value());

    // Test: startOrder while another is active -> rejected
    auto r2 = manager.startOrder("Jane Doe", "555-5678");
    CHECK(!r2.success);

    // Test: add product with stock 5, add 2, add 3 merged into ONE line with qty 5
    auto r3 = manager.addByBarcode("100001", 2000);  // 2 pcs
    CHECK(r3.success);
    CHECK_EQ(manager.current()->lines.size(), static_cast<size_t>(1));

    auto r4 = manager.addByBarcode("100001", 3000);  // 3 pcs
    CHECK(r4.success);
    CHECK_EQ(manager.current()->lines.size(), static_cast<size_t>(1));
    CHECK_EQ(manager.current()->lines[0].quantity, 5000);

    // Test: add 1 more rejected (stock 5, already have 5)
    auto r5 = manager.addByBarcode("100001", 1000);
    CHECK(!r5.success);

    // Test: unknown barcode rejected
    auto r6 = manager.addByBarcode("999999", 1000);
    CHECK(!r6.success);

    // Test: quantity 0 rejected
    auto r7 = manager.addByBarcode("100001", 0);
    CHECK(!r7.success);

    // Test: negative quantity rejected
    auto r8 = manager.addByBarcode("100001", -1000);
    CHECK(!r8.success);

    // Test: no active order rejected
    OrderManager manager2(catalog, nullptr, dateTimeProvider);
    auto r9 = manager2.addByBarcode("100001", 1000);
    CHECK(!r9.success);

    // Test: zero-stock product rejected
    auto r10 = manager.addByBarcode("100003", 1000);
    CHECK(!r10.success);

    // Test: weight product 1.25 kg (1250) works
    auto r11 = manager.addByBarcode("100002", 1250);
    CHECK(r11.success);

    // Test: count product with decimals (e.g., 1500) rejected
    auto r12 = manager.addByBarcode("100001", 1500);  // 1.5 pcs
    CHECK(!r12.success);

    // Test: setQuantity works and re-checks stock
    auto r13 = manager.setQuantity("P001", 4000);  // Set to 4 pcs
    CHECK(r13.success);
    CHECK_EQ(manager.current()->lines[0].quantity, 4000);

    // Test: setQuantity must not double count (check against NEW total)
    auto r14 = manager.setQuantity("P001", 5000);  // Set to 5 pcs
    CHECK(r14.success);

    // Test: setQuantity with insufficient stock rejected
    auto r15 = manager.setQuantity("P001", 6000);  // Set to 6 pcs
    CHECK(!r15.success);

    // Test: setQuantity for non-existent line rejected
    auto r16 = manager.setQuantity("P999", 1000);
    CHECK(!r16.success);

    // Test: removeLine works
    auto r17 = manager.removeLine("P002");
    CHECK(r17.success);
    CHECK_EQ(manager.current()->lines.size(), static_cast<size_t>(1));

    // Test: removeLine for non-existent line rejected
    auto r18 = manager.removeLine("P999");
    CHECK(!r18.success);

    // Test: hold -> heldOrders has it, current empty
    auto r19 = manager.hold();
    CHECK(r19.success);
    CHECK(!manager.current().has_value());
    CHECK_EQ(manager.heldOrders().size(), static_cast<size_t>(1));

    std::string heldOrderId = manager.heldOrders()[0].id;

    // Test: resume works
    auto r20 = manager.resume(heldOrderId);
    CHECK(r20.success);
    CHECK(manager.current().has_value());
    CHECK_EQ(manager.heldOrders().size(), static_cast<size_t>(0));

    // Test: resume fails while another order is active
    OrderManager manager3(catalog, nullptr, dateTimeProvider);
    manager3.startOrder("Test", "123");
    auto r21 = manager3.resume(heldOrderId);
    CHECK(!r21.success);

    // Test: resume of a non-held id fails
    auto r22 = manager.resume("99999999");
    CHECK(!r22.success);

    // Test: cancel -> status archived, stock unchanged, current empty
    auto r23 = manager.cancel();
    CHECK(r23.success);
    CHECK(!manager.current().has_value());

    // Test: fake store inside test verifies save is called after each change
    {
        MockOrderStore mockStore;
        OrderManager managerWithStore(catalog, &mockStore, dateTimeProvider);

        auto ms1 = managerWithStore.startOrder("Alice", "111-222");
        CHECK(ms1.success);
        CHECK_EQ(mockStore.saveCallCount, 1);

        auto ms2 = managerWithStore.addByBarcode("100001", 1000);
        CHECK(ms2.success);
        CHECK_EQ(mockStore.saveCallCount, 2);

        auto ms3 = managerWithStore.setQuantity("P001", 2000);
        CHECK(ms3.success);
        CHECK_EQ(mockStore.saveCallCount, 3);

        auto ms4 = managerWithStore.removeLine("P001");
        CHECK(ms4.success);
        CHECK_EQ(mockStore.saveCallCount, 4);

        auto ms5 = managerWithStore.hold();
        CHECK(ms5.success);
        CHECK_EQ(mockStore.saveCallCount, 5);

        auto held = managerWithStore.heldOrders();
        CHECK_EQ(held.size(), static_cast<size_t>(1));
        if (!held.empty()) {
            std::string mockHeldId = held[0].id;
            auto ms6 = managerWithStore.resume(mockHeldId);
            CHECK(ms6.success);
            CHECK_EQ(mockStore.saveCallCount, 6);
        }

        auto ms7 = managerWithStore.cancel();
        CHECK(ms7.success);
        CHECK_EQ(mockStore.saveCallCount, 7);
    }

    return testSummary("test_order_manager");
}
