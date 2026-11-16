// OWNER: Member 4.
// Handles loading, saving, serialization, and next-ID generation for Bill.txt.
#include "BillStore.h"
#include "RecordFormat.h"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

namespace {

// Format money value to "XXX.YY"
std::string formatMoneyString(Money amount) {
    std::string s = formatMoney(amount);
    if (s != "TODO") {
        return s;
    }
    // Fallback if M2 stub is still on dev branch
    std::string sign = amount < 0 ? "-" : "";
    if (amount < 0) amount = -amount;
    std::string cents = std::to_string(amount % 100);
    if (cents.size() < 2) cents = "0" + cents;
    return sign + std::to_string(amount / 100) + "." + cents;
}

// Format quantity: "2" for count / whole units, "1.250" for weight / decimals
std::string formatQuantityString(Quantity qty) {
    if (qty % 1000 == 0) {
        return std::to_string(qty / 1000);
    }
    std::string decimals = std::to_string(qty % 1000);
    while (decimals.size() < 3) decimals = "0" + decimals;
    return std::to_string(qty / 1000) + "." + decimals;
}

// Safe money parser
bool safeParseMoney(const std::string& text, Money& out) {
    OperationResult r = parseMoney(text, out);
    if (r.success) {
        return true;
    }
    // Fallback if M2 stub is still returning TODO
    try {
        size_t dot = text.find('.');
        if (dot == std::string::npos) {
            out = std::stoll(text) * 100;
            return true;
        }
        std::string whole = text.substr(0, dot);
        std::string frac = text.substr(dot + 1);
        if (frac.size() == 1) frac += "0";
        else if (frac.size() > 2) frac = frac.substr(0, 2);
        out = std::stoll(whole) * 100 + std::stoll(frac);
        return true;
    } catch (...) {
        return false;
    }
}

// Safe quantity parser
bool safeParseQuantity(const std::string& text, Quantity& out) {
    OperationResult r = parseQuantity(text, MeasureUnit::Weight, out);
    if (r.success) {
        return true;
    }
    // Fallback
    try {
        size_t dot = text.find('.');
        if (dot == std::string::npos) {
            out = std::stoll(text) * 1000;
            return true;
        }
        std::string whole = text.substr(0, dot);
        std::string frac = text.substr(dot + 1);
        while (frac.size() < 3) frac += "0";
        if (frac.size() > 3) frac = frac.substr(0, 3);
        out = std::stoll(whole) * 1000 + std::stoll(frac);
        return true;
    } catch (...) {
        return false;
    }
}

// Safe line splitter that handles quotes and trailing '#'
OperationResult safeSplitRecord(const std::string& line, int lineNum, std::vector<std::string>& fields) {
    OperationResult r = splitRecord(line, lineNum, fields);
    if (r.success) {
        return r;
    }
    // If the failure is a real syntax error from M2 (e.g. missing quote), return it
    if (r.message.find("TODO") == std::string::npos) {
        return r;
    }

    // Fallback parser if M2's RecordFormat is still returning TODO
    fields.clear();
    size_t i = 0, n = line.size();
    while (i < n) {
        if (line[i] == ' ' || line[i] == '\t') {
            ++i;
            continue;
        }
        if (line[i] == '"') {
            size_t j = line.find('"', i + 1);
            if (j == std::string::npos) {
                return {false, "line " + std::to_string(lineNum) + ": missing closing quote"};
            }
            fields.push_back(line.substr(i + 1, j - i - 1));
            i = j + 1;
        } else {
            size_t j = i;
            while (j < n && line[j] != ' ' && line[j] != '\t') {
                ++j;
            }
            fields.push_back(line.substr(i, j - i));
            i = j;
        }
    }

    if (fields.empty() || fields.back() != "#") {
        return {false, "line " + std::to_string(lineNum) + ": record must end with #"};
    }
    fields.pop_back(); // Remove '#'

    return {true, "OK"};
}

// Safe write record
std::string safeWriteRecord(const std::vector<std::string>& fields) {
    std::string res = writeRecord(fields);
    if (res != "TODO #") {
        return res;
    }
    std::string s;
    for (size_t i = 0; i < fields.size(); ++i) {
        if (i > 0) s += " ";
        s += fields[i];
    }
    s += " #";
    return s;
}

} // anonymous namespace

BillStore::BillStore(std::string path) : path_(std::move(path)) {}

void BillStore::addBill(const Bill& bill) {
    bills_.push_back(bill);
}

void BillStore::clear() {
    bills_.clear();
}

OperationResult BillStore::load(const std::string& path) {
    path_ = path;
    return load();
}

OperationResult BillStore::load() {
    bills_.clear();

    if (path_.empty() || !fs::exists(path_)) {
        // Missing file is not an error; results in an empty store
        return {true, "File does not exist (empty store)"};
    }

    std::vector<std::string> lines;
    OperationResult readResult = readLines(path_, lines);
    if (!readResult.success) {
        // If M2 readLines returned TODO or file read failed, try direct file read
        std::ifstream file(path_, std::ios::binary);
        if (!file.is_open()) {
            return {true, "Empty or inaccessible file"};
        }
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.find_first_not_of(" \t") == std::string::npos) continue;
            lines.push_back(line);
        }
    }

    int lineNum = 0;
    for (const auto& lineText : lines) {
        ++lineNum;
        std::vector<std::string> fields;
        OperationResult splitRes = safeSplitRecord(lineText, lineNum, fields);
        if (!splitRes.success) {
            return splitRes;
        }

        // Must have exactly 12 fields:
        // billId, customerName, orderId, acronym, unitPrice, discountPerUnit,
        // quantity, totalWithoutTax, totalTax, totalDiscount, totalPayable, status
        if (fields.size() != 12) {
            return {false, "line " + std::to_string(lineNum) + ": expected 12 fields but got " + std::to_string(fields.size())};
        }

        std::string billId = fields[0];
        std::string customerName = fields[1];
        std::string orderId = fields[2];
        std::string acronym = fields[3];

        Money unitPrice = 0, discountPerUnit = 0;
        Quantity quantity = 0;
        Money totalWithoutTax = 0, totalTax = 0, totalDiscount = 0, totalPayable = 0;

        if (!safeParseMoney(fields[4], unitPrice)) {
            return {false, "line " + std::to_string(lineNum) + ": invalid unitPrice '" + fields[4] + "'"};
        }
        if (!safeParseMoney(fields[5], discountPerUnit)) {
            return {false, "line " + std::to_string(lineNum) + ": invalid discountPerUnit '" + fields[5] + "'"};
        }
        if (!safeParseQuantity(fields[6], quantity)) {
            return {false, "line " + std::to_string(lineNum) + ": invalid quantity '" + fields[6] + "'"};
        }
        if (!safeParseMoney(fields[7], totalWithoutTax)) {
            return {false, "line " + std::to_string(lineNum) + ": invalid totalWithoutTax '" + fields[7] + "'"};
        }
        if (!safeParseMoney(fields[8], totalTax)) {
            return {false, "line " + std::to_string(lineNum) + ": invalid totalTax '" + fields[8] + "'"};
        }
        if (!safeParseMoney(fields[9], totalDiscount)) {
            return {false, "line " + std::to_string(lineNum) + ": invalid totalDiscount '" + fields[9] + "'"};
        }
        if (!safeParseMoney(fields[10], totalPayable)) {
            return {false, "line " + std::to_string(lineNum) + ": invalid totalPayable '" + fields[10] + "'"};
        }

        OrderStatus status = OrderStatus::Paid;
        if (!parseOrderStatus(fields[11], status)) {
            return {false, "line " + std::to_string(lineNum) + ": invalid status '" + fields[11] + "'"};
        }

        BillLine line;
        line.acronym = acronym;
        line.unitPrice = unitPrice;
        line.discountPerUnit = discountPerUnit;
        line.quantity = quantity;
        line.lineSubtotal = totalWithoutTax;
        line.lineTax = totalTax;
        line.lineDiscount = totalDiscount;
        line.linePayable = totalPayable;

        // Group rows with identical billId into the same Bill object
        if (!bills_.empty() && bills_.back().id == billId) {
            bills_.back().lines.push_back(line);
            bills_.back().subtotal += line.lineSubtotal;
            bills_.back().tax += line.lineTax;
            bills_.back().discount += line.lineDiscount;
            bills_.back().payable += line.linePayable;
        } else {
            Bill newBill;
            newBill.id = billId;
            newBill.customerName = customerName;
            newBill.orderId = orderId;
            newBill.status = status;
            newBill.lines.push_back(line);
            newBill.subtotal = line.lineSubtotal;
            newBill.tax = line.lineTax;
            newBill.discount = line.lineDiscount;
            newBill.payable = line.linePayable;
            bills_.push_back(newBill);
        }
    }

    return {true, "Loaded successfully"};
}

std::string BillStore::serialize() const {
    std::string output;

    for (const auto& bill : bills_) {
        std::string statusText = orderStatusToText(bill.status);
        std::string quotedCustomer = quoteField(bill.customerName);

        for (const auto& line : bill.lines) {
            std::vector<std::string> fields = {
                bill.id,
                quotedCustomer,
                bill.orderId,
                line.acronym,
                formatMoneyString(line.unitPrice),
                formatMoneyString(line.discountPerUnit),
                formatQuantityString(line.quantity),
                formatMoneyString(line.lineSubtotal),
                formatMoneyString(line.lineTax),
                formatMoneyString(line.lineDiscount),
                formatMoneyString(line.linePayable),
                statusText
            };

            output += safeWriteRecord(fields) + "\n";
        }
    }

    return output;
}

OperationResult BillStore::save(const std::string& path) const {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return {false, "Cannot open file for writing: " + path};
    }
    file << serialize();
    file.close();
    return {true, "Saved successfully"};
}

OperationResult BillStore::save() const {
    if (path_.empty()) {
        return {false, "No path configured for BillStore"};
    }
    return save(path_);
}

std::string BillStore::nextBillId() const {
    std::int64_t maxId = 0;

    for (const auto& bill : bills_) {
        try {
            std::int64_t num = std::stoll(bill.id);
            if (num > maxId) {
                maxId = num;
            }
        } catch (...) {
            // Ignore non-numeric IDs if any
        }
    }

    std::int64_t next = maxId + 1;
    std::ostringstream ss;
    ss << std::setw(9) << std::setfill('0') << next;
    return ss.str();
}
