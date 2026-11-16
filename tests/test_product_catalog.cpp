#include <filesystem>
#include <fstream>
#include <iostream>
#include "ProductCatalog.h"

static int failed = 0;
#define CHECK(c) do { if (!(c)) { std::cout << "FAIL line " << __LINE__ << ": " #c "\n"; ++failed; } } while (0)

static void writeFile(const std::string& p, const std::string& s) { std::ofstream(p, std::ios::binary) << s; }
static std::string readAll(const std::string& p) { std::ifstream f(p, std::ios::binary); return {std::istreambuf_iterator<char>(f), {}}; }

int main()
{
    ProductCatalog c;
    CHECK(!c.load("no_such_file.txt").success);

    writeFile("pc_empty.txt", "");
    CHECK(c.load("pc_empty.txt").success && c.all().empty());

    writeFile("pc_mixed.txt",
        "101 123456 MILKP \"Milk Powder 400g\" count 450.00 20.00 10.00 12 \"img/a.bmp\" #\n"
        "102 123456 DUPBC \"Dup barcode\" count 1.00 0.00 0.00 1 \"\" #\n"
        "103 012345 BADBC \"Zero first\" count 1.00 0.00 0.00 1 \"\" #\n"
        "104 200145 SUGAR \"Sugar\" weight 160.00 5.00 0.00 25.500 \"\" #\n"
        "105 300000 TOOLONGACR \"X\" count 1.00 0.00 0.00 1 \"\" #\n"
        "106 400000 NEG \"Neg\" count -5.00 0.00 0.00 1 \"\" #\n"
        "107 500000 NOHASH \"No hash\" count 1.00 0.00 0.00 1 \"\"\n"
        "108 600000 KG \"Bad unit\" liters 1.00 0.00 0.00 1 \"\" #\n");
    CHECK(c.load("pc_mixed.txt").success);
    CHECK(c.all().size() == 2);
    CHECK(c.warnings().size() == 6);
    CHECK(c.findByBarcode("123456") && c.findByBarcode("123456")->name == "Milk Powder 400g");
    CHECK(c.findById("104") && c.findById("104")->available == 25500);
    CHECK(!c.findByBarcode("999999"));

    CHECK(c.save("pc_saved.txt").success);
    ProductCatalog c2;
    CHECK(c2.load("pc_saved.txt").success && c2.warnings().empty() && c2.all().size() == 2);
    CHECK(readAll("pc_saved.txt") == c2.serialize());

    CHECK(c2.reduceStock("101", 2000).success && c2.findById("101")->available == 10000);
    CHECK(!c2.reduceStock("101", 999000).success);
    CHECK(c2.findById("101")->available == 10000);
    CHECK(!c2.reduceStock("nope", 1000).success);
    CHECK(!c2.reduceStock("101", 0).success);

    ProductCatalog s;
    if (s.load("sample_data/Product.txt").success && !s.all().empty()) {
        CHECK(s.warnings().empty());
        CHECK(s.save("pc_sample_copy.txt").success);
        CHECK(readAll("pc_sample_copy.txt") == readAll("sample_data/Product.txt"));
    } else {
        std::cout << "SKIP: sample_data/Product.txt not found or empty from this folder\n";
    }

    for (auto f : {"pc_empty.txt", "pc_mixed.txt", "pc_saved.txt", "pc_sample_copy.txt"})
        std::filesystem::remove(f);
    std::cout << (failed ? "FAILED\n" : "All product catalog tests passed\n");
    return failed ? 1 : 0;
}