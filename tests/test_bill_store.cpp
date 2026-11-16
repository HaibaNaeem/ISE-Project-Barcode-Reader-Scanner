// Tests for Member 4 BillStore module
#include "TestHelpers.h"
#include "BillStore.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

static void cleanFolder(const fs::path& p) {
    std::error_code ec;
    fs::remove_all(p, ec);
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

static void testLoadExistingRowsAndNextId() {
    fs::path tempDir = fs::temp_directory_path() / "test_bill_store_load";
    cleanFolder(tempDir);
    fs::create_directories(tempDir);

    fs::path billFile = tempDir / "Bill.txt";
    {
        std::ofstream out(billFile, std::ios::binary);
        // Canonical sample row from specification
        out << "000000031 \"Ali Khan\" 00000714 MILKP 450.00 10.00 2 900.00 40.00 20.00 920.00 paid #\n";
    }

    BillStore store(billFile.string());
    OperationResult r = store.load();
    CHECK(r.success);
    CHECK_EQ(store.bills().size(), 1);

    const Bill& b = store.bills()[0];
    CHECK_EQ(b.id, std::string("000000031"));
    CHECK_EQ(b.customerName, std::string("Ali Khan"));
    CHECK_EQ(b.orderId, std::string("00000714"));
    CHECK_EQ(b.lines.size(), 1);

    const BillLine& l = b.lines[0];
    CHECK_EQ(l.acronym, std::string("MILKP"));
    CHECK_EQ(l.unitPrice, 45000);
    CHECK_EQ(l.discountPerUnit, 1000);
    CHECK_EQ(l.quantity, 2000);
    CHECK_EQ(l.lineSubtotal, 90000);
    CHECK_EQ(l.lineTax, 4000);
    CHECK_EQ(l.lineDiscount, 2000);
    CHECK_EQ(l.linePayable, 92000);

    // Next ID should be 000000032
    CHECK_EQ(store.nextBillId(), std::string("000000032"));

    cleanFolder(tempDir);
}

static void testMultiLineBillGrouping() {
    fs::path tempDir = fs::temp_directory_path() / "test_bill_store_multi";
    cleanFolder(tempDir);
    fs::create_directories(tempDir);

    fs::path billFile = tempDir / "Bill.txt";
    {
        std::ofstream out(billFile, std::ios::binary);
        out << "000000031 \"Ali Khan\" 00000714 MILKP 450.00 10.00 2 900.00 40.00 20.00 920.00 paid #\n";
        out << "000000031 \"Ali Khan\" 00000714 BREAD 120.00 5.00 1 120.00 0.00 5.00 115.00 paid #\n";
    }

    BillStore store(billFile.string());
    OperationResult r = store.load();
    CHECK(r.success);
    CHECK_EQ(store.bills().size(), 1); // Grouped into 1 bill

    const Bill& b = store.bills()[0];
    CHECK_EQ(b.lines.size(), 2);
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
        out << "000000001 \"Customer One\" 00000101 ITEM1 100.00 0.00 2 200.00 20.00 0.00 220.00 paid #\n";
        out << "000000002 \"Customer Two\" 00000102 ITEM2 50.00 5.00 1 50.00 0.00 5.00 45.00 paid #\n";
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
    CHECK_EQ(store2.bills()[1].id, store1.bills()[1].id);
    CHECK_EQ(store2.bills()[1].payable, store1.bills()[1].payable);

    cleanFolder(tempDir);
}

static void testMalformedRowWithLineNumber() {
    fs::path tempDir = fs::temp_directory_path() / "test_bill_store_malformed";
    cleanFolder(tempDir);
    fs::create_directories(tempDir);

    // File with bad line 2 (missing closing quote)
    fs::path badFile = tempDir / "BadQuote.txt";
    {
        std::ofstream out(badFile, std::ios::binary);
        out << "000000001 \"Valid Customer\" 00000101 ITEM1 100.00 0.00 1 100.00 0.00 0.00 100.00 paid #\n";
        out << "000000002 \"Unclosed Customer 00000102 ITEM2 50.00 0.00 1 50.00 0.00 0.00 50.00 paid #\n";
    }

    BillStore store(badFile.string());
    OperationResult r = store.load();
    CHECK(!r.success);
    // Message should contain the line number
    CHECK(r.message.find("line 2") != std::string::npos);

    // File with bad field count on line 1
    fs::path badFields = tempDir / "BadFields.txt";
    {
        std::ofstream out(badFields, std::ios::binary);
        out << "000000001 \"Incomplete Line\" 00000101 ITEM1 100.00 paid #\n";
    }

    BillStore store2(badFields.string());
    OperationResult r2 = store2.load();
    CHECK(!r2.success);
    CHECK(r2.message.find("line 1") != std::string::npos);

    cleanFolder(tempDir);
}

int main() {
    testEmptyAndMissingFileNextId();
    testLoadExistingRowsAndNextId();
    testMultiLineBillGrouping();
    testRoundTripSaveAndLoad();
    testMalformedRowWithLineNumber();
    return testSummary("test_bill_store");
}

