#pragma once
// OWNER: Member 4. Bill.txt persistence and ID generation.
#include <string>
#include <vector>
#include "CommonTypes.h"
#include "Bill.h"

class BillStore {
public:
    BillStore() = default;
    explicit BillStore(std::string path);

    // Load bills from the configured path. Missing file results in an empty store (not an error).
    // Malformed rows return failure with line number and reason.
    OperationResult load();
    OperationResult load(const std::string& path);

    // Save bills to the configured path.
    OperationResult save() const;
    OperationResult save(const std::string& path) const;

    // Produces the entire Bill.txt file content as text (used by CheckoutService and save).
    std::string serialize() const;

    // Generates the next 9-digit zero-padded Bill ID (e.g. "000000001", "000000032").
    std::string nextBillId() const;

    // Adds a new bill to the in-memory store.
    void addBill(const Bill& bill);

    // Clears all in-memory bills.
    void clear();

    const std::string& path() const { return path_; }
    void setPath(const std::string& path) { path_ = path; }

    std::vector<Bill>& bills() { return bills_; }
    const std::vector<Bill>& bills() const { return bills_; }

private:
    std::string path_;
    std::vector<Bill> bills_;
};
