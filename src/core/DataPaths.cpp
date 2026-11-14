#include "DataPaths.h"
#include <filesystem>
#include <fstream>
#include <system_error>

namespace fs = std::filesystem;

DataPaths::DataPaths(std::string exeDir) : exeDir_(std::move(exeDir)) {}

std::string DataPaths::dataDir() const          { return (fs::path(exeDir_) / "data").generic_string(); }
std::string DataPaths::productFile() const      { return (fs::path(dataDir()) / "Product.txt").generic_string(); }
std::string DataPaths::orderFile() const        { return (fs::path(dataDir()) / "Order.txt").generic_string(); }
std::string DataPaths::orderDetailFile() const  { return (fs::path(dataDir()) / "OrderDetail.txt").generic_string(); }
std::string DataPaths::billFile() const         { return (fs::path(dataDir()) / "Bill.txt").generic_string(); }

std::string DataPaths::resolve(const std::string& relativePath) const {
    fs::path p(relativePath);
    if (p.is_absolute()) return p.generic_string();
    return (fs::path(exeDir_) / p).generic_string();
}

OperationResult DataPaths::ensureDataFolder() const {
    std::error_code ec;
    fs::create_directories(dataDir(), ec);
    if (ec) return {false, "Cannot create data folder: " + ec.message()};

    const char* names[] = {"Product.txt", "Order.txt", "OrderDetail.txt", "Bill.txt"};
    int created = 0;
    for (const char* name : names) {
        fs::path target = fs::path(dataDir()) / name;
        if (fs::exists(target, ec)) continue;                       // never overwrite real data
        fs::path sample = fs::path(exeDir_) / "sample_data" / name;
        if (fs::exists(sample, ec)) {
            fs::copy_file(sample, target, ec);
            if (ec) return {false, std::string("Cannot copy ") + name + ": " + ec.message()};
        } else {
            std::ofstream out(target, std::ios::binary);            // empty file
            if (!out) return {false, std::string("Cannot create ") + name};
        }
        ++created;
    }
    return {true, created ? "Data folder prepared" : "Data folder ready"};
}
