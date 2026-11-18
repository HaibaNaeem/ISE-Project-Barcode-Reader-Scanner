// Tests for Member 4 CheckoutService (All-or-Nothing checkout orchestration)
#include "TestHelpers.h"
#include "CheckoutService.h"
#include "ProductCatalog.h"
#include "OrderStore.h"
#include "BillStore.h"
#include "OrderManager.h"
#include "Pricing.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace {

// Clean temporary test directory
void cleanFolder(const fs::path& p) {
    std::error_code ec;
    fs::remove_all(p, ec);
}

// Read whole file as string
std::string readFileContent(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) return "";
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

// // TEMP workaround inside Member 4's test file:
// Mirrors OrderManager memory layout to populate test order while M3's startOrder() is a stub.
struct TestOrderManagerAccessor {
    const ProductCatalog* catalog;
    OrderStore* store;
    std::optional<Order> current;
    std::function<DateTime()> clock;
};

void setTestActiveOrder(OrderManager& mgr, const Order& ord) {
    reinterpret_cast<TestOrderManagerAccessor*>(&mgr)->current = ord;
}

} // anonymous namespace

static void testSuccessfulPayment() {
    fs::path tempDir = fs::temp_directory_path() / "test_checkout_success";
    cleanFolder(tempDir);
    fs::create_directories(tempDir);

    fs::path prodFile = tempDir / "Product.txt";
    fs::path ordFile  = tempDir / "Order.txt";
    fs::path detFile  = tempDir / "OrderDetail.txt";
    fs::path billFile = tempDir / "Bill.txt";

    // Setup Products in memory
    Product milk;
    milk.id = "101";
    milk.acronym = "MILKP";
    milk.name = "Milk Powder";
    milk.unitPrice = 45000;        // 450.00
    milk.taxPerUnit = 2000;        // 20.00
    milk.discountPerUnit = 1000;   // 10.00
    milk.available = 10000;        // 10 pcs available

    Product bread;
    bread.id = "102";
    bread.acronym = "BREAD";
    bread.name = "Bread 500g";
    bread.unitPrice = 12000;       // 120.00
    bread.taxPerUnit = 0;
    bread.discountPerUnit = 500;   // 5.00
    bread.available = 5000;        // 5 pcs available

    ProductCatalog catalog({milk, bread});
    OrderStore orders(ordFile.string(), detFile.string());
    BillStore bills(billFile.string());

    CheckoutService service(catalog, orders, bills, prodFile.string());
    OrderManager manager(catalog, &orders);

    // Active order: 2 Milk and 1 Bread
    Order testOrder;
    testOrder.id = "00000714";
    testOrder.customerName = "Ali Khan";
    testOrder.date = "2026-11-16";
    testOrder.time = "14:05:09";
    testOrder.lines = {
        {"101", "", 2000}, // 2 Milk
        {"102", "", 1000}  // 1 Bread
    };
    setTestActiveOrder(manager, testOrder);

    PaymentInfo payment;
    payment.mode = PaymentMode::Cash;
    payment.merchant = "";

    Bill outBill;
    OperationResult r = service.pay(manager, payment, outBill);

    // 1. Transaction succeeded
    CHECK(r.success);

    // 2. Bill correctly calculated
    CHECK_EQ(outBill.lines.size(), 2);
    CHECK_EQ(outBill.subtotal, 102000); // 900.00 + 120.00 = 1020.00
    CHECK_EQ(outBill.tax, 4000);        // 40.00 + 0 = 40.00
    CHECK_EQ(outBill.discount, 2500);   // 20.00 + 5.00 = 25.00
    CHECK_EQ(outBill.payable, 103500);  // 1020 + 40 - 25 = 1035.00 PKR
    CHECK(outBill.status == OrderStatus::Paid);
    CHECK_EQ(outBill.id, std::string("000000001"));

    // 3. In-memory stock reduced
    auto p1 = catalog.findById("101");
    CHECK(p1.has_value());
    CHECK_EQ(p1->available, 8000); // 10000 - 2000 = 8000

    auto p2 = catalog.findById("102");
    CHECK(p2.has_value());
    CHECK_EQ(p2->available, 4000); // 5000 - 1000 = 4000

    // 4. Current order cleared in OrderManager
    CHECK(!manager.current().has_value());

    // 5. BillStore has the new bill
    CHECK_EQ(bills.bills().size(), 1);

    // 6. All four files exist on disk and are not empty
    CHECK(fs::exists(prodFile));
    CHECK(fs::exists(ordFile));
    CHECK(fs::exists(detFile));
    CHECK(fs::exists(billFile));

    // Reload bill from disk to verify persistence
    BillStore reloaded(billFile.string());
    CHECK(reloaded.load().success);
    CHECK_EQ(reloaded.bills().size(), 1);
    CHECK_EQ(reloaded.bills()[0].payable, 103500);

    cleanFolder(tempDir);
}

static void testInsufficientStockRejected() {
    fs::path tempDir = fs::temp_directory_path() / "test_checkout_insufficient";
    cleanFolder(tempDir);
    fs::create_directories(tempDir);

    fs::path prodFile = tempDir / "Product.txt";
    fs::path ordFile  = tempDir / "Order.txt";
    fs::path detFile  = tempDir / "OrderDetail.txt";
    fs::path billFile = tempDir / "Bill.txt";

    Product item;
    item.id = "201";
    item.name = "Limited Item";
    item.unitPrice = 10000;
    item.available = 1000; // Only 1 available

    ProductCatalog catalog({item});
    OrderStore orders(ordFile.string(), detFile.string());
    BillStore bills(billFile.string());

    CheckoutService service(catalog, orders, bills, prodFile.string());
    OrderManager manager(catalog, &orders);

    // Order asks for 2 units (2000 thousandths) -> exceeds stock!
    Order testOrder;
    testOrder.id = "ORD01";
    testOrder.lines = { {"201", "", 2000} };
    setTestActiveOrder(manager, testOrder);

    PaymentInfo payment;
    payment.mode = PaymentMode::Cash;

    Bill outBill;
    OperationResult r = service.pay(manager, payment, outBill);

    // Must fail
    CHECK(!r.success);
    CHECK(r.message.find("Insufficient stock") != std::string::npos);

    // Stock must be completely unchanged
    auto prod = catalog.findById("201");
    CHECK(prod.has_value());
    CHECK_EQ(prod->available, 1000);

    // Order must NOT be cleared
    CHECK(manager.current().has_value());

    // No bills added
    CHECK_EQ(bills.bills().size(), 0);

    cleanFolder(tempDir);
}

static void testInvalidPaymentRejected() {
    fs::path tempDir = fs::temp_directory_path() / "test_checkout_invalid_pay";
    cleanFolder(tempDir);
    fs::create_directories(tempDir);

    Product p;
    p.id = "301";
    p.unitPrice = 5000;
    p.available = 5000;

    ProductCatalog catalog({p});
    OrderStore orders((tempDir / "O.txt").string(), (tempDir / "D.txt").string());
    BillStore bills((tempDir / "B.txt").string());
    CheckoutService service(catalog, orders, bills, (tempDir / "P.txt").string());
    OrderManager manager(catalog, &orders);

    Order testOrder;
    testOrder.id = "ORD02";
    testOrder.lines = { {"301", "", 1000} };
    setTestActiveOrder(manager, testOrder);

    // Card payment WITHOUT merchant name -> must fail
    PaymentInfo cardNoMerchant;
    cardNoMerchant.mode = PaymentMode::Card;
    cardNoMerchant.merchant = "";

    Bill outBill;
    OperationResult r1 = service.pay(manager, cardNoMerchant, outBill);
    CHECK(!r1.success);

    // Cash payment WITH merchant name -> must fail
    PaymentInfo cashWithMerchant;
    cashWithMerchant.mode = PaymentMode::Cash;
    cashWithMerchant.merchant = "Bank";

    OperationResult r2 = service.pay(manager, cashWithMerchant, outBill);
    CHECK(!r2.success);

    // None payment mode -> must fail
    PaymentInfo nonePayment;
    nonePayment.mode = PaymentMode::None;
    OperationResult r3 = service.pay(manager, nonePayment, outBill);
    CHECK(!r3.success);

    // Stock unchanged
    CHECK_EQ(catalog.findById("301")->available, 5000);

    cleanFolder(tempDir);
}

static void testForcedWriteFailureAllOrNothing() {
    // CRITICAL SPEC REQUIREMENT:
    // After a forced write failure, all four files AND in-memory stock are exactly as before the payment.
    fs::path tempDir = fs::temp_directory_path() / "test_checkout_forced_fail";
    cleanFolder(tempDir);
    fs::create_directories(tempDir);

    fs::path prodFile = tempDir / "Product.txt";
    fs::path ordFile  = tempDir / "Order.txt";
    fs::path detFile  = tempDir / "OrderDetail.txt";
    fs::path billFile = tempDir / "Bill.txt";

    // Write initial file contents
    {
        std::ofstream p(prodFile); p << "INITIAL_PRODUCT_CONTENT\n";
        std::ofstream o(ordFile);  o << "INITIAL_ORDER_CONTENT\n";
        std::ofstream d(detFile);  d << "INITIAL_DETAIL_CONTENT\n";
        std::ofstream b(billFile); b << "INITIAL_BILL_CONTENT\n";
    }

    std::string initProd = readFileContent(prodFile.string());
    std::string initOrd  = readFileContent(ordFile.string());
    std::string initDet  = readFileContent(detFile.string());
    std::string initBill = readFileContent(billFile.string());

    Product item;
    item.id = "401";
    item.acronym = "ATOM";
    item.name = "Atomic Item";
    item.unitPrice = 10000;
    item.available = 5000; // Initial stock: 5 units

    ProductCatalog catalog({item});
    OrderStore orders(ordFile.string(), detFile.string());
    BillStore bills(billFile.string());

    CheckoutService service(catalog, orders, bills, prodFile.string());
    OrderManager manager(catalog, &orders);

    Order testOrder;
    testOrder.id = "ORD_FAIL_TEST";
    testOrder.lines = { {"401", "", 2000} };
    setTestActiveOrder(manager, testOrder);

    PaymentInfo payment;
    payment.mode = PaymentMode::Cash;

    // SIMULATE FORCED WRITE FAILURE
    service.simulateWriteFailure(true);

    Bill outBill;
    OperationResult r = service.pay(manager, payment, outBill);

    // 1. Payment must report failure
    CHECK(!r.success);

    // 2. In-memory stock numbers must be EXACTLY as before the payment (still 5000)
    auto prodAfter = catalog.findById("401");
    CHECK(prodAfter.has_value());
    CHECK_EQ(prodAfter->available, 5000);

    // 3. Manager active order must NOT be cleared
    CHECK(manager.current().has_value());
    CHECK_EQ(manager.current()->id, std::string("ORD_FAIL_TEST"));

    // 4. BillStore must have NO new bills
    CHECK_EQ(bills.bills().size(), 0);

    // 5. ALL FOUR FILES on disk must be byte-for-byte EXACTLY as before the payment
    CHECK_EQ(readFileContent(prodFile.string()), initProd);
    CHECK_EQ(readFileContent(ordFile.string()),  initOrd);
    CHECK_EQ(readFileContent(detFile.string()),  initDet);
    CHECK_EQ(readFileContent(billFile.string()), initBill);

    cleanFolder(tempDir);
}

int main() {
    testSuccessfulPayment();
    testInsufficientStockRejected();
    testInvalidPaymentRejected();
    testForcedWriteFailureAllOrNothing();
    return testSummary("test_checkout");
}

