#pragma once
// OWNER: Member 5. Tiny test helper: CHECK macros, pass/fail count, exit code.
//
//   #include "TestHelpers.h"
//   int main() {
//       CHECK(1 + 1 == 2);
//       CHECK_EQ(formatMoney(12550), "125.50");
//       return testSummary("test_units");     // prints "test_units  Passed: N  Failed: M", returns 0 or 1
//   }
#include <iostream>
#include <sstream>
#include <string>

inline int& testPassed() { static int n = 0; return n; }
inline int& testFailed() { static int n = 0; return n; }

inline void testRecord(bool ok, const char* expr, const char* file, int line) {
    if (ok) { ++testPassed(); return; }
    ++testFailed();
    std::cout << "FAIL  " << file << ":" << line << "  " << expr << "\n";
}

template <typename A, typename B>
void testRecordEq(const A& a, const B& b, const char* ea, const char* eb, const char* file, int line) {
    if (a == b) { ++testPassed(); return; }
    ++testFailed();
    std::ostringstream os;
    os << "FAIL  " << file << ":" << line << "  " << ea << " == " << eb << "   (got [" << a << "] expected [" << b << "])";
    std::cout << os.str() << "\n";
}

#define CHECK(cond)       testRecord(static_cast<bool>(cond), #cond, __FILE__, __LINE__)
#define CHECK_EQ(a, b)    testRecordEq((a), (b), #a, #b, __FILE__, __LINE__)

inline int testSummary(const std::string& name) {
    std::cout << name << "  Passed: " << testPassed() << "  Failed: " << testFailed() << "\n";
    return testFailed() == 0 ? 0 : 1;
}
