// Tests for Member 4 BillStore module
// Bill.txt format: ONE LINE = ONE BILL (items inside the line, separated by ';')
#include "TestHelpers.h"
#include "BillStore.h"
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

static void cleanFolder(const fs::path& p) {
    std::error_code ec;
    fs::remove_all(p, ec);
}

static std::string readWholeFile(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static void testEmptyAndMissingFileNextId() {
    fs::path tempDir = fs::temp_directory_path() / "test_bill_store_empty";
    cleanFolder(tempDir);
    fs::create_directories(tempDir);

    fs::path missingFile = tempDir / "NonExistentBill.txt";
    BillStore store(missingFile.string());

    OperationResult r = store.load();
    CHECK(r.success);
    CHECK_EQ(store.bills().size(), 0);
    CHECK_EQ(store.nextBillId(), std::string("000000001"));

    cleanFolder(tempDir);
}

static void testLoadSingleItemBillAndNextId() {
    fs::path tempDir = fs::temp_directory_path() / "test_bill_store_load";
    cleanFolder(tempDir);
    fs::create_directories(tempDir);

    fs::path billFile = tempDir / "Bill.txt";
    {
        std::ofstream out(billFile, std::ios::binary);
        out << "000000031 \"Ali Khan\" 00000714 MILKP,450.00,10.00,2 900.00 40.00 20.00 920.00 paid #\n";
    }

    BillStore store(billFile.string());
    OperationResult r = store.load();
    CHECK(r.success);
    CHECK_EQ(store.bills().size(), 1);

    const Bill& b = store.bills()[0];
    CHECK_EQ(b.id, std::string("000000031"));
    CHECK_EQ(b.customerName, std::string("Ali Khan"));
    CHECK_EQ(b.orderId, std::string("00000714"));
    CHECK(b.status == OrderStatus::Paid);
    CHECK_EQ(b.lines.size(), 1);

    const BillLine& l = b.lines[0];
    CHECK_EQ(l.acronym, std::string("MILKP"));
    CHECK_EQ(l.unitPrice, 45000);
    CHECK_EQ(l.discountPerUnit, 1000);
    CHECK_EQ(l.quantity, 2000);

    // Bill totals come straight from the file
    CHECK_EQ(b.subtotal, 90000);
    CHECK_EQ(b.tax, 4000);
    CHECK_EQ(b.discount, 2000);
    CHECK_EQ(b.payable, 92000);

    CHECK_EQ(store.nextBillId(), std::string("000000032"));

    cleanFolder(tempDir);
}

static void testMultiItemBillOnOneLine() {
    fs::path tempDir = fs::temp_directory_path() / "test_bill_store_multi";
    cleanFolder(tempDir);
    fs::create_directories(tempDir);

    fs::path billFile = tempDir / "Bill.txt";
    {
        std::ofstream out(billFile, std::ios::binary);
        out << "000000031 \"Ali Khan\" 00000714 MILKP,450.00,10.00,2;BREAD,120.00,5.00,1 "
               "1020.00 40.00 25.00 1035.00 paid #\n";
    }

    BillStore store(billFile.string());
    OperationResult r = store.load();
    CHECK(r.success);
    CHECK_EQ(store.bills().size(), 1);   // one line = one bill

    const Bill& b = store.bills()[0];
    CHECK_EQ(b.lines.size(), 2);
    CHECK_EQ(b.lines[0].acronym, std::string("MILKP"));
    CHECK_EQ(b.lines[0].quantity, 2000);
    CHECK_EQ(b.lines[1].acronym, std::string("BREAD"));
    CHECK_EQ(b.lines[1].unitPrice, 12000);
    CHECK_EQ(b.lines[1].discountPerUnit, 500);
    CHECK_EQ(b.lines[1].quantity, 1000);

    CHECK_EQ(b.subtotal, 102000);
    CHECK_EQ(b.tax, 4000);
    CHECK_EQ(b.discount, 2500);
    CHECK_EQ(b.payable, 103500);

    cleanFolder(tempDir);
}

static void testRoundTripSaveAndLoad() {
    fs::path tempDir = fs::temp_directory_path() / "test_bill_store_roundtrip";
    cleanFolder(tempDir);
    fs::create_directories(tempDir);

    fs::path originalFile = tempDir / "Original.txt";
    fs::path savedFile = tempDir / "Saved.txt";

    {
        std::ofstream out(originalFile, std::ios::binary);
        out << "000000001 \"Customer One\" 00000101 ITEM1,100.00,0.00,2 200.00 20.00 0.00 220.00 paid #\n";
        out << "000000002 \"Customer Two\" 00000102 ITEM2,50.00,5.00,1;ITEM3,10.00,0.00,1.250 "
               "62.50 0.00 5.00 57.50 paid #\n";
    }

    BillStore store1(originalFile.string());
    CHECK(store1.load().success);
    CHECK(store1.save(savedFile.string()).success);

    BillStore store2(savedFile.string());
    CHECK(store2.load().success);

    CHECK_EQ(store2.bills().size(), 2);
    CHECK_EQ(store2.bills()[0].id, store1.bills()[0].id);
    CHECK_EQ(store2.bills()[0].customerName, store1.bills()[0].customerName);
    CHECK_EQ(store2.bills()[0].payable, store1.bills()[0].payable);
    CHECK_EQ(store2.bills()[1].lines.size(), 2);
    CHECK_EQ(store2.bills()[1].lines[1].quantity, 1250);   // weight item survives
    CHECK_EQ(store2.bills()[1].payable, store1.bills()[1].payable);

    // load then save gives an identical file
    CHECK_EQ(readWholeFile(savedFile), readWholeFile(originalFile));

    cleanFolder(tempDir);
}

static void testMalformedRowWithLineNumber() {
    fs::path tempDir = fs::temp_directory_path() / "test_bill_store_malformed";
    cleanFolder(tempDir);
    fs::create_directories(tempDir);

    // Bad line 2 (missing closing quote)
    fs::path badFile = tempDir / "BadQuote.txt";
    {
        std::ofstream out(badFile, std::ios::binary);
        out << "000000001 \"Valid Customer\" 00000101 ITEM1,100.00,0.00,1 100.00 0.00 0.00 100.00 paid #\n";
        out << "000000002 \"Unclosed Customer 00000102 ITEM2,50.00,0.00,1 50.00 0.00 0.00 50.00 paid #\n";
    }
    BillStore store(badFile.string());
    OperationResult r = store.load();
    CHECK(!r.success);
    CHECK(r.message.find("line 2") != std::string::npos);

    // Wrong field count on line 1 (expected 9)
    fs::path badFields = tempDir / "BadFields.txt";
    {
        std::ofstream out(badFields, std::ios::binary);
        out << "000000001 \"Incomplete Line\" 00000101 ITEM1,100.00,0.00,1 paid #\n";
    }
    BillStore store2(badFields.string());
    OperationResult r2 = store2.load();
    CHECK(!r2.success);
    CHECK(r2.message.find("line 1") != std::string::npos);

    // Item with a missing part on line 1 (only 3 parts instead of 4)
    fs::path badItem = tempDir / "BadItem.txt";
    {
        std::ofstream out(badItem, std::ios::binary);
        out << "000000001 \"Ali\" 00000101 ITEM1,100.00,0.00 100.00 0.00 0.00 100.00 paid #\n";
    }
    BillStore store3(badItem.string());
    OperationResult r3 = store3.load();
    CHECK(!r3.success);
    CHECK(r3.message.find("line 1") != std::string::npos);

    cleanFolder(tempDir);
}

int main() {
    testEmptyAndMissingFileNextId();
    testLoadSingleItemBillAndNextId();
    testMultiItemBillOnOneLine();
    testRoundTripSaveAndLoad();
    testMalformedRowWithLineNumber();
    return testSummary("test_bill_store");
}