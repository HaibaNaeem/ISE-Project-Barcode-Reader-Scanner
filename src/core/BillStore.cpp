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

constexpr char kItemSeparator  = ';';   // between two products
constexpr char kFieldSeparator = ',';   // between the parts of one product
constexpr size_t kFieldsPerBillLine = 9;
constexpr size_t kPartsPerItem = 4;

// Splits "a;b;c" into {"a","b","c"} (simple helper, no tricks)
std::vector<std::string> splitText(const std::string& text, char separator) {
    std::vector<std::string> parts;
    std::string current;
    for (char c : text) {
        if (c == separator) {
            parts.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }
    parts.push_back(current);
    return parts;
}

// Reads the <items> field into BillLine objects.
OperationResult parseItems(const std::string& text, int lineNum, std::vector<BillLine>& lines) {
    lines.clear();
    for (const std::string& itemText : splitText(text, kItemSeparator)) {
        std::vector<std::string> parts = splitText(itemText, kFieldSeparator);
        if (parts.size() != kPartsPerItem) {
            return {false, "line " + std::to_string(lineNum) + ": bad item '" + itemText +
                           "' (expected acronym,price,discount,quantity)"};
        }

        BillLine line;
        line.acronym = parts[0];
        if (!safeParseMoney(parts[1], line.unitPrice)) {
            return {false, "line " + std::to_string(lineNum) + ": invalid unitPrice '" + parts[1] + "'"};
        }
        if (!safeParseMoney(parts[2], line.discountPerUnit)) {
            return {false, "line " + std::to_string(lineNum) + ": invalid discountPerUnit '" + parts[2] + "'"};
        }
        if (!safeParseQuantity(parts[3], line.quantity)) {
            return {false, "line " + std::to_string(lineNum) + ": invalid quantity '" + parts[3] + "'"};
        }
        lines.push_back(line);
    }
    return {true, "OK"};
}

// Builds the <items> field from BillLine objects.
std::string buildItemsField(const std::vector<BillLine>& lines) {
    std::string text;
    for (size_t i = 0; i < lines.size(); ++i) {
        if (i > 0) text += kItemSeparator;
        text += lines[i].acronym;
        text += kFieldSeparator;
        text += formatMoneyString(lines[i].unitPrice);
        text += kFieldSeparator;
        text += formatMoneyString(lines[i].discountPerUnit);
        text += kFieldSeparator;
        text += formatQuantityString(lines[i].quantity);
    }
    return text;
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

        // One line = one bill, exactly 9 fields:
        // 0 billId, 1 customerName, 2 orderId, 3 items,
        // 4 totalWithoutTax, 5 totalTax, 6 totalDiscount, 7 totalPayable, 8 status
        if (fields.size() != kFieldsPerBillLine) {
            return {false, "line " + std::to_string(lineNum) + ": expected " +
                           std::to_string(kFieldsPerBillLine) + " fields but got " +
                           std::to_string(fields.size())};
        }

        Bill bill;
        bill.id = fields[0];
        bill.customerName = fields[1];
        bill.orderId = fields[2];

        OperationResult itemsRes = parseItems(fields[3], lineNum, bill.lines);
        if (!itemsRes.success) {
            return itemsRes;
        }

        if (!safeParseMoney(fields[4], bill.subtotal)) {
            return {false, "line " + std::to_string(lineNum) + ": invalid totalWithoutTax '" + fields[4] + "'"};
        }
        if (!safeParseMoney(fields[5], bill.tax)) {
            return {false, "line " + std::to_string(lineNum) + ": invalid totalTax '" + fields[5] + "'"};
        }
        if (!safeParseMoney(fields[6], bill.discount)) {
            return {false, "line " + std::to_string(lineNum) + ": invalid totalDiscount '" + fields[6] + "'"};
        }
        if (!safeParseMoney(fields[7], bill.payable)) {
            return {false, "line " + std::to_string(lineNum) + ": invalid totalPayable '" + fields[7] + "'"};
        }
        if (!parseOrderStatus(fields[8], bill.status)) {
            return {false, "line " + std::to_string(lineNum) + ": invalid status '" + fields[8] + "'"};
        }

        bills_.push_back(bill);
    }

    return {true, "Loaded successfully"};
}
std::string BillStore::serialize() const {
    std::string output;

    for (const auto& bill : bills_) {
        // One line per bill, all products inside the items field
        std::vector<std::string> fields = {
            bill.id,
            quoteField(bill.customerName),
            bill.orderId,
            buildItemsField(bill.lines),
            formatMoneyString(bill.subtotal),
            formatMoneyString(bill.tax),
            formatMoneyString(bill.discount),
            formatMoneyString(bill.payable),
            orderStatusToText(bill.status)
        };
        output += safeWriteRecord(fields) + "\n";
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
