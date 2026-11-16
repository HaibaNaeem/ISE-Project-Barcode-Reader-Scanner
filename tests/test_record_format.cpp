#include <filesystem>
#include <fstream>
#include <iostream>
#include "RecordFormat.h"

static int failed = 0;
#define CHECK(c) do { if (!(c)) { std::cout << "FAIL line " << __LINE__ << ": " #c "\n"; ++failed; } } while (0)

static std::string readAll(const std::string& p) { std::ifstream f(p, std::ios::binary); return {std::istreambuf_iterator<char>(f), {}}; }

int main()
{
    std::vector<std::string> f;

    auto a = splitRecord("101 123456 MILKP \"Milk Powder 400g\" count 450.00 #", 1, f);
    CHECK(a.success && f.size() == 6 && f[3] == "Milk Powder 400g");

    a = splitRecord("00000714 2026-11-16 \"Ali Khan\" \"\" on-hold #", 2, f);
    CHECK(a.success && f.size() == 5 && f[3].empty());

    auto c = splitRecord("101 123456", 7, f);
    CHECK(!c.success && c.message.find("line 7") == 0);
    CHECK(!splitRecord("101 \"Milk #", 3, f).success);
    CHECK(!splitRecord("", 4, f).success);

    CHECK(writeRecord({"101", quoteField("Milk Powder"), "12"}) == "101 \"Milk Powder\" 12 #");
    CHECK(quoteField("") == "\"\"");

    namespace fs = std::filesystem;
    fs::create_directories("tx_test");

    std::vector<std::string> lines;
    CHECK(!readLines("tx_test/none.txt", lines).success);
    { std::ofstream("tx_test/empty.txt"); }
    CHECK(readLines("tx_test/empty.txt", lines).success && lines.empty());
    { std::ofstream("tx_test/two.txt") << "a #\n\n   \nb #\n"; }
    CHECK(readLines("tx_test/two.txt", lines).success && lines.size() == 2);

    { std::ofstream("tx_test/a.txt") << "old A"; std::ofstream("tx_test/b.txt") << "old B"; }

    FileTransaction ok;
    ok.add("tx_test/a.txt", "new A");
    ok.add("tx_test/b.txt", "new B");
    CHECK(ok.commit().success);
    CHECK(readAll("tx_test/a.txt") == "new A" && readAll("tx_test/b.txt") == "new B");

    FileTransaction bad;
    bad.add("tx_test/a.txt", "X A");
    bad.add("tx_test/b.txt", "X B");
    bad.simulateFailure(true);
    CHECK(!bad.commit().success);
    CHECK(readAll("tx_test/a.txt") == "new A" && readAll("tx_test/b.txt") == "new B");
    CHECK(!fs::exists("tx_test/a.txt.tmp") && !fs::exists("tx_test/b.txt.tmp"));

    fs::remove_all("tx_test");
    std::cout << (failed ? "FAILED\n" : "All record format tests passed\n");
    return failed ? 1 : 0;
}