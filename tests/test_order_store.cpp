#include "OrderStore.h"
#include "Order.h"
#include "TestHelpers.h"
#include <filesystem>
#include <fstream>

int main() {
    // Test: Order ID is 8 digits
    std::vector<Order> emptyOrders;
    std::string id1 = OrderStore::generateNextOrderId(emptyOrders, 15);
    CHECK_EQ(id1.length(), static_cast<size_t>(8));
    CHECK_EQ(id1, std::string("00000115"));

    // Test: correct day suffix
    std::string id2 = OrderStore::generateNextOrderId(emptyOrders, 5);
    CHECK_EQ(id2.substr(6, 2), std::string("05"));

    std::string id3 = OrderStore::generateNextOrderId(emptyOrders, 31);
    CHECK_EQ(id3.substr(6, 2), std::string("31"));

    // Test: counter increments
    std::vector<Order> orders;
    Order o1;
    o1.id = "00000115";
    o1.status = OrderStatus::Paid;
    orders.push_back(o1);

    std::string id4 = OrderStore::generateNextOrderId(orders, 15);
    CHECK_EQ(id4, std::string("00000215"));

    orders.push_back(o1);  // Add another with same ID
    std::string id5 = OrderStore::generateNextOrderId(orders, 15);
    CHECK_EQ(id5, std::string("00000215"));

    // Test: IDs never repeat when archived/paid/on-hold orders exist
    Order o2;
    o2.id = "00000520";
    o2.status = OrderStatus::Archived;
    orders.push_back(o2);

    Order o3;
    o3.id = "00000320";
    o3.status = OrderStatus::OnHold;
    orders.push_back(o3);

    std::string id6 = OrderStore::generateNextOrderId(orders, 20);
    CHECK_EQ(id6, std::string("00000620"));

    // Test: crash recovery - in-progress becomes on-hold
    std::vector<Order> orders2;
    Order o4;
    o4.id = "00000115";
    o4.status = OrderStatus::InProgress;
    orders2.push_back(o4);

    Order o5;
    o5.id = "00000215";
    o5.status = OrderStatus::Paid;
    orders2.push_back(o5);

    OrderStore::recoverInProgressOrders(orders2);
    CHECK(orders2[0].status == OrderStatus::OnHold);
    CHECK(orders2[1].status == OrderStatus::Paid);

    // Test: orphan helper
    std::vector<Order> orders3;
    Order o6;
    o6.id = "00000115";
    orders3.push_back(o6);

    std::vector<std::string> detailLines = {
        "00000115 101 123456 2",
        "00000215 102 234567 3",
        "00000115 103 345678 1"
    };

    auto orphans = OrderStore::findOrphanDetailLines(orders3, detailLines);
    CHECK_EQ(orphans.size(), static_cast<size_t>(1));
    CHECK_EQ(orphans[0], std::string("00000215 102 234567 3"));

    // File round-trip test
    std::string testOrderPath = "test_orders.txt";
    std::string testDetailPath = "test_details.txt";

    {
        OrderStore store(testOrderPath, testDetailPath);
        Order ord;
        ord.id = "00000714";
        ord.date = "2026-11-16";
        ord.time = "14:05:09";
        ord.customerName = "Ali Khan";
        ord.customerContact = "";
        ord.status = OrderStatus::InProgress; // will test crash recovery upon load
        ord.paymentAmount = 0;
        ord.mode = PaymentMode::None;
        ord.merchant = "";
        ord.lines.push_back(OrderLine{"101", "123456", 2000});
        store.orders().push_back(ord);

        auto r = store.save();
        CHECK(r.success);
    }

    {
        // Reload in new store instance
        OrderStore store2(testOrderPath, testDetailPath);
        auto r = store2.load();
        CHECK(r.success);
        CHECK_EQ(store2.orders().size(), static_cast<size_t>(1));
        CHECK_EQ(store2.orders()[0].id, std::string("00000714"));
        CHECK_EQ(store2.orders()[0].customerName, std::string("Ali Khan"));
        CHECK(store2.orders()[0].status == OrderStatus::OnHold);
        CHECK_EQ(store2.orders()[0].lines.size(), static_cast<size_t>(1));
        CHECK_EQ(store2.orders()[0].lines[0].productId, std::string("101"));
        CHECK_EQ(store2.orders()[0].lines[0].quantity, 2000);
    }

    std::filesystem::remove(testOrderPath);
    std::filesystem::remove(testDetailPath);

    return testSummary("test_order_store");
}
