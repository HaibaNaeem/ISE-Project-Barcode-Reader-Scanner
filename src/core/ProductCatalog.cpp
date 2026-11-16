#include "ProductCatalog.h"
#include "CodeValidator.h"
#include "RecordFormat.h"
#include <sstream>

namespace {
bool parseProductRow(const std::vector<std::string>& f, Product& p, std::string& err)
{
    if (f.size() != 10) { err = "expected 10 fields, found " + std::to_string(f.size()); return false; }
    p.id = f[0]; p.barcode = f[1]; p.acronym = f[2]; p.name = f[3];
    if (p.id.empty()) { err = "empty id"; return false; }
    if (!isValidSixDigitCode(p.barcode)) { err = "invalid barcode '" + p.barcode + "'"; return false; }
    if (p.acronym.empty() || p.acronym.size() > 8) { err = "acronym must be 1 to 8 characters"; return false; }
    if (p.name.empty()) { err = "empty name"; return false; }

    OperationResult r = parseMeasureUnit(f[4], p.unit);
    if (!r.success) { err = r.message; return false; }
    r = parseMoney(f[5], p.unitPrice);        if (!r.success) { err = r.message; return false; }
    r = parseMoney(f[6], p.taxPerUnit);       if (!r.success) { err = r.message; return false; }
    r = parseMoney(f[7], p.discountPerUnit);  if (!r.success) { err = r.message; return false; }
    r = parseQuantity(f[8], p.unit, p.available);
    if (!r.success) { err = r.message; return false; }
    p.barcodeImagePath = f[9];
    return true;
}
}

ProductCatalog::ProductCatalog(std::vector<Product> products) : products_(std::move(products)) {}

OperationResult ProductCatalog::load(const std::string& path)
{
    products_.clear();
    warnings_.clear();

    std::vector<std::string> lines;
    OperationResult r = readLines(path, lines);
    if (!r.success) return r;      // file missing

    int lineNo = 0;
    for (auto& line : lines) {
        ++lineNo;                  // blank lines readLines mein skip hoti hain, isliye number approx
        std::vector<std::string> f;
        OperationResult s = splitRecord(line, lineNo, f);
        if (!s.success) { warnings_.push_back(s.message + " (row skipped)"); continue; }

        Product p;
        std::string err;
        if (!parseProductRow(f, p, err)) {
            warnings_.push_back("line " + std::to_string(lineNo) + ": " + err + " (row skipped)");
            continue;
        }
        bool dup = false;
        for (auto& e : products_)
            if (e.barcode == p.barcode || e.id == p.id) { dup = true; break; }
        if (dup) {
            warnings_.push_back("line " + std::to_string(lineNo) + ": duplicate id or barcode (row skipped)");
            continue;
        }
        products_.push_back(p);
    }

    OperationResult ok;
    ok.success = true;
    ok.message = std::to_string(products_.size()) + " products loaded";
    if (!warnings_.empty()) {
        ok.message += ", " + std::to_string(warnings_.size()) + " rows skipped";
        for (auto& w : warnings_) ok.message += "\n" + w;
    }
    return ok;
}

std::string ProductCatalog::serialize() const
{
    std::ostringstream out;
    for (auto& p : products_) {
        out << writeRecord({p.id, p.barcode, p.acronym, quoteField(p.name),
                            measureUnitToText(p.unit),
                            formatMoney(p.unitPrice), formatMoney(p.taxPerUnit), formatMoney(p.discountPerUnit),
                            formatQuantity(p.available, p.unit), quoteField(p.barcodeImagePath)})
            << "\n";
    }
    return out.str();
}

OperationResult ProductCatalog::save(const std::string& path) const
{
    FileTransaction t;
    t.add(path, serialize());
    return t.commit();
}

std::optional<Product> ProductCatalog::findByBarcode(const std::string& barcode) const
{
    for (auto& p : products_) if (p.barcode == barcode) return p;
    return std::nullopt;
}

std::optional<Product> ProductCatalog::findById(const std::string& id) const
{
    for (auto& p : products_) if (p.id == id) return p;
    return std::nullopt;
}

OperationResult ProductCatalog::reduceStock(const std::string& productId, Quantity amount)
{
    OperationResult r;
    if (amount <= 0) { r.message = "Amount must be greater than zero."; return r; }
    for (auto& p : products_) {
        if (p.id != productId) continue;
        if (p.available < amount) { r.message = "Not enough stock for " + p.name + "."; return r; }
        p.available -= amount;
        r.success = true;
        r.message = "Stock updated.";
        return r;
    }
    r.message = "Product not found.";
    return r;
}