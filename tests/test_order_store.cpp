#include "../src/core/OrderStore.h"
#include "../src/core/Order.h"
#include "TestHelpers.h"
#include <iostream>

int main() {
    // Test: Order ID is 8 digits
    std::vector<Order> emptyOrders;
    std::string id1 = OrderStore::generateNextOrderId(emptyOrders, 15);
    CHECK(id1.length() == 8, "Order ID is 8 digits");
    CHECK(id1 == "00000115", "Empty list gives counter 000001 with day 15");

    // Test: correct day suffix
    std::string id2 = OrderStore::generateNextOrderId(emptyOrders, 5);
    CHECK(id2.substr(6, 2) == "05", "Day suffix is correct (05)");

    std::string id3 = OrderStore::generateNextOrderId(emptyOrders, 31);
    CHECK(id3.substr(6, 2) == "31", "Day suffix is correct (31)");

    // Test: counter increments
    std::vector<Order> orders;
    Order o1;
    o1.id = "00000115";
    o1.status = OrderStatus::Paid;
    orders.push_back(o1);

    std::string id4 = OrderStore::generateNextOrderId(orders, 15);
    CHECK(id4 == "00000215", "Counter increments to 000002");

    orders.push_back(o1);  // Add another with same ID
    std::string id5 = OrderStore::generateNextOrderId(orders, 15);
    CHECK(id5 == "00000215", "Counter still 000002 (max is 1)");

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
    CHECK(id6 == "00000620", "Counter increments to 000006 (max was 5)");

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
    CHECK(orders2[0].status == OrderStatus::OnHold, "in-progress becomes on-hold");
    CHECK(orders2[1].status == OrderStatus::Paid, "paid order unchanged");

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
    CHECK(orphans.size() == 1, "one orphan line found");
    CHECK(orphans[0] == "00000215 102 234567 3", "correct orphan line");

    // TODO(after Member 2): round-trip, missing file, empty file, malformed row

    std::cout << "\nPassed: " << passed << "  Failed: " << failed << "\n";
    return failed == 0 ? 0 : 1;
}
