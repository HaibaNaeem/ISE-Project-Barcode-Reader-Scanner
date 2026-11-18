// OWNER: Member 3. Order.txt + OrderDetail.txt are always loaded and saved together.
#include "OrderStore.h"
#include "RecordFormat.h"
#include "Units.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace {

Quantity parseQtyHelper(const std::string& s) {
    size_t dot = s.find('.');
    if (dot == std::string::npos) {
        try {
            return std::stoll(s) * 1000;
        } catch (...) {
            return 0;
        }
    }
    std::string ip = s.substr(0, dot);
    std::string fp = s.substr(dot + 1);
    while (fp.size() < 3) fp.push_back('0');
    if (fp.size() > 3) fp = fp.substr(0, 3);
    try {
        std::int64_t whole = ip.empty() ? 0 : std::stoll(ip);
        std::int64_t frac = fp.empty() ? 0 : std::stoll(fp);
        return whole * 1000 + frac;
    } catch (...) {
        return 0;
    }
}

std::string formatQtyHelper(Quantity q) {
    if (q % 1000 == 0) {
        return std::to_string(q / 1000);
    }
    return formatQuantity(q, MeasureUnit::Weight);
}

} // anonymous namespace

OrderStore::OrderStore(std::string orderPath, std::string detailPath)
    : orderPath_(std::move(orderPath)), detailPath_(std::move(detailPath)) {}

OperationResult OrderStore::load() {
    orders_.clear();

    if (!orderPath_.empty()) {
        std::vector<std::string> orderLines;
        OperationResult r = readLines(orderPath_, orderLines);
        if (!r.success) {
            // If file does not exist yet, it's ok (empty store)
        } else {
            int lineNo = 0;
            for (const auto& line : orderLines) {
                ++lineNo;
                std::vector<std::string> f;
                OperationResult s = splitRecord(line, lineNo, f);
                if (!s.success || f.size() != 9) continue;

                Order ord;
                ord.id = f[0];
                ord.date = f[1];
                ord.time = f[2];
                ord.customerName = f[3];
                ord.customerContact = f[4];

                parseOrderStatus(f[5], ord.status);
                // Crash recovery: any in-progress order left after crash comes back as on-hold
                if (ord.status == OrderStatus::InProgress) {
                    ord.status = OrderStatus::OnHold;
                }

                parseMoney(f[6], ord.paymentAmount);
                parsePaymentMode(f[7], ord.mode);
                ord.merchant = f[8];

                orders_.push_back(ord);
            }
        }
    }

    if (!detailPath_.empty()) {
        std::vector<std::string> detailLines;
        OperationResult r = readLines(detailPath_, detailLines);
        if (r.success) {
            int lineNo = 0;
            for (const auto& line : detailLines) {
                ++lineNo;
                std::vector<std::string> f;
                OperationResult s = splitRecord(line, lineNo, f);
                if (!s.success) continue;

                std::string orderId;
                std::string productId;
                std::string barcode;
                Quantity qty = 0;

                if (f.size() == 4) {
                    orderId = f[0];
                    productId = f[1];
                    barcode = f[2];
                    qty = parseQtyHelper(f[3]);
                } else if (f.size() == 3) {
                    orderId = f[0];
                    productId = f[1];
                    barcode = "";
                    qty = parseQtyHelper(f[2]);
                } else {
                    continue;
                }

                // Attach to matching order if exists (orphan detail rows skipped)
                for (auto& ord : orders_) {
                    if (ord.id == orderId) {
                        ord.lines.push_back(OrderLine{productId, barcode, qty});
                        break;
                    }
                }
            }
        }
    }

    OperationResult res;
    res.success = true;
    res.message = std::to_string(orders_.size()) + " orders loaded";
    return res;
}

std::string OrderStore::serializeOrders() const {
    std::string text;
    for (const auto& ord : orders_) {
        std::vector<std::string> fields = {
            ord.id,
            ord.date.empty() ? "2026-11-16" : ord.date,
            ord.time.empty() ? "00:00:00" : ord.time,
            quoteField(ord.customerName),
            quoteField(ord.customerContact),
            orderStatusToText(ord.status),
            formatMoney(ord.paymentAmount),
            paymentModeToText(ord.mode),
            quoteField(ord.merchant)
        };
        text += writeRecord(fields) + "\n";
    }
    return text;
}

std::string OrderStore::serializeDetails() const {
    std::string text;
    for (const auto& ord : orders_) {
        for (const auto& line : ord.lines) {
            std::string bc = line.barcode.empty() ? line.productId : line.barcode;
            std::vector<std::string> fields = {
                ord.id,
                line.productId,
                bc,
                formatQtyHelper(line.quantity)
            };
            text += writeRecord(fields) + "\n";
        }
    }
    return text;
}

OperationResult OrderStore::save() const {
    FileTransaction tx;
    if (!orderPath_.empty()) {
        tx.add(orderPath_, serializeOrders());
    }
    if (!detailPath_.empty()) {
        tx.add(detailPath_, serializeDetails());
    }
    return tx.commit();
}

OperationResult OrderStore::save(const Order& order) {
    bool found = false;
    for (auto& ord : orders_) {
        if (ord.id == order.id) {
            ord = order;
            found = true;
            break;
        }
    }
    if (!found) {
        orders_.push_back(order);
    }
    return save();
}

std::string OrderStore::nextOrderId(int dayOfMonth) const {
    return generateNextOrderId(orders_, dayOfMonth);
}

std::string OrderStore::generateNextOrderId(const std::vector<Order>& existingOrders, int dayOfMonth) {
    int maxCounter = 0;
    for (const auto& ord : existingOrders) {
        if (ord.id.length() == 8) {
            try {
                int c = std::stoi(ord.id.substr(0, 6));
                if (c > maxCounter) {
                    maxCounter = c;
                }
            } catch (...) {}
        }
    }
    int nextCounter = maxCounter + 1;
    std::ostringstream ss;
    ss << std::setw(6) << std::setfill('0') << nextCounter
       << std::setw(2) << std::setfill('0') << dayOfMonth;
    return ss.str();
}

void OrderStore::recoverInProgressOrders(std::vector<Order>& orders) {
    for (auto& ord : orders) {
        if (ord.status == OrderStatus::InProgress) {
            ord.status = OrderStatus::OnHold;
        }
    }
}

std::vector<std::string> OrderStore::findOrphanDetailLines(const std::vector<Order>& orders,
                                                           const std::vector<std::string>& detailLines) {
    std::vector<std::string> orphanLines;
    for (const auto& line : detailLines) {
        std::stringstream ss(line);
        std::string orderId;
        if (ss >> orderId) {
            bool found = false;
            for (const auto& ord : orders) {
                if (ord.id == orderId) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                orphanLines.push_back(line);
            }
        }
    }
    return orphanLines;
}
