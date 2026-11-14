#pragma once
// OWNER: Member 4. Bill.txt.
#include <string>
#include <vector>
#include "CommonTypes.h"
#include "Bill.h"

class BillStore {
public:
    BillStore() = default;
    explicit BillStore(std::string path);

    OperationResult load();
    OperationResult save() const;
    std::string serialize() const;               // whole Bill.txt as text (used by CheckoutService)
    const std::string& path() const { return path_; }

    std::string nextBillId() const;              // 9 digits, last used + 1, padded with zeros
    std::vector<Bill>& bills() { return bills_; }
    const std::vector<Bill>& bills() const { return bills_; }

private:
    std::string path_;
    std::vector<Bill> bills_;
};
