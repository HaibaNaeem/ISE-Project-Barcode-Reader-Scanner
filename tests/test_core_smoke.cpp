// OWNER: Member 5. Proves the skeleton builds and the pieces M5 implemented work.
// Includes every core header, so a broken header is found here first.
#include <filesystem>
#include <system_error>
#include <fstream>
#include "TestHelpers.h"
#include "Units.h"
#include "RecordFormat.h"
#include "Product.h"
#include "ProductCatalog.h"
#include "StockRules.h"
#include "Order.h"
#include "OrderStore.h"
#include "OrderManager.h"
#include "Bill.h"
#include "Pricing.h"
#include "BillStore.h"
#include "CheckoutService.h"
#include "DataPaths.h"

namespace fs = std::filesystem;

// Reads the first line and CLOSES the file again (Windows cannot delete a file that is still open).
static std::string firstLine(const std::string& path) {
    std::ifstream in(path);
    std::string line;
    std::getline(in, line);
    return line;
}

static void removeFolder(const fs::path& p) {
    std::error_code ec;                // error_code version never throws
    fs::remove_all(p, ec);
}

static void testStockRule() {
    CHECK(hasEnoughStock(0, 5, 5));      // exactly the stock: allowed
    CHECK(hasEnoughStock(3, 2, 5));
    CHECK(!hasEnoughStock(5, 1, 5));     // one more than stock
    CHECK(!hasEnoughStock(0, 6, 5));
}

static void testEnumHelpers() {
    OrderStatus s; PaymentMode m;
    for (const char* t : {"in-progress", "on-hold", "paid", "archived"}) {
        CHECK(parseOrderStatus(t, s));
        CHECK_EQ(orderStatusToText(s), std::string(t));
    }
    for (const char* t : {"cash", "card", "wallet", "none"}) {
        CHECK(parsePaymentMode(t, m));
        CHECK_EQ(paymentModeToText(m), std::string(t));
    }
    CHECK(!parseOrderStatus("done", s));
    CHECK(!parsePaymentMode("bitcoin", m));
}

static void testCatalogInMemory() {
    Product milk; milk.id = "101"; milk.barcode = "123456"; milk.acronym = "MILKP";
    milk.name = "Milk Powder 400g"; milk.unitPrice = 45000; milk.available = 12000;
    Product rice; rice.id = "102"; rice.barcode = "234567"; rice.unit = MeasureUnit::Weight;
    ProductCatalog cat({milk, rice});
    CHECK(cat.findByBarcode("123456").has_value());
    CHECK_EQ(cat.findByBarcode("123456")->name, std::string("Milk Powder 400g"));
    CHECK(cat.findById("102").has_value());
    CHECK(!cat.findByBarcode("999999").has_value());
    CHECK(!cat.findById("nope").has_value());
    CHECK(cat.reduceStock("101", 2000).success);
    CHECK_EQ(cat.findById("101")->available, Quantity(10000));
    CHECK(!cat.reduceStock("101", 99000).success);          // never below zero
    CHECK_EQ(cat.findById("101")->available, Quantity(10000));
}

static void testDataPaths() {
    fs::path root = fs::temp_directory_path() / "pos_smoke_datapaths";
    removeFolder(root);
    fs::create_directories(root / "sample_data");
    { std::ofstream(root / "sample_data" / "Product.txt") << "sample product\n"; }

    DataPaths paths(root.generic_string());
    OperationResult r = paths.ensureDataFolder();
    CHECK(r.success);
    CHECK(fs::exists(paths.productFile()));
    CHECK(fs::exists(paths.orderFile()));
    CHECK(fs::exists(paths.orderDetailFile()));
    CHECK(fs::exists(paths.billFile()));

    CHECK_EQ(firstLine(paths.productFile()), std::string("sample product"));          // copied from sample_data
    CHECK_EQ(fs::file_size(paths.orderFile()), std::uintmax_t(0));   // no sample -> empty file

    { std::ofstream(paths.productFile()) << "changed\n"; }
    CHECK(paths.ensureDataFolder().success);                // second run must not overwrite
    CHECK_EQ(firstLine(paths.productFile()), std::string("changed"));

    CHECK_EQ(paths.resolve("sample_data/barcodes/1.bmp"), (root / "sample_data/barcodes/1.bmp").generic_string());
    removeFolder(root);
}

static void testStubsLinkAndDoNotCrash() {
    // Only checks that the stubs can be constructed and called. Real behaviour belongs in each owner's tests.
    ProductCatalog cat;
    OrderStore orders("o.txt", "d.txt");
    BillStore bills("b.txt");
    OrderManager manager(cat, &orders);
    CheckoutService checkout(cat, orders, bills, "p.txt");
    Bill b; Order o;
    (void)buildBill(o, cat, "000000001");
    CHECK_EQ(changeDue(1000, 1500), Money(500));
    CHECK_EQ(changeDue(1000, 500), Money(0));
    CHECK(!manager.current().has_value());
}

int main() {
    testStockRule();
    testEnumHelpers();
    testCatalogInMemory();
    testDataPaths();
    testStubsLinkAndDoNotCrash();
    return testSummary("test_core_smoke");
}
