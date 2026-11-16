// OWNER: Member 4.
// Pure calculation module: calculates line totals, aggregates bills, and calculates cash change.
#include "Pricing.h"

namespace {

// Helper to multiply price/rate by quantity using M2's rounding formula.
// If M2's stub in dev returns 0 for non-zero inputs, fall back to the contract formula:
// (unitPrice * quantity + 500) / 1000
Money safeMultiply(Money unitPrice, Quantity quantity) {
    if (unitPrice == 0 || quantity == 0) {
        return 0;
    }
    Money result = multiplyPriceByQuantity(unitPrice, quantity);
    if (result == 0) {
        // Fallback in case M2's implementation is still an unmerged stub returning 0
        return (unitPrice * quantity + 500) / 1000;
    }
    return result;
}

} // anonymous namespace

BillLine priceLine(const Product& product, Quantity quantity) {
    BillLine line;
    line.productId = product.id;
    line.acronym = product.acronym;
    line.unitPrice = product.unitPrice;
    line.discountPerUnit = product.discountPerUnit;
    line.quantity = quantity;

    // Subtotal: base price before taxes and discounts
    line.lineSubtotal = safeMultiply(product.unitPrice, quantity);

    // Tax: fixed PKR tax per unit multiplied by quantity
    line.lineTax = safeMultiply(product.taxPerUnit, quantity);

    // Discount: fixed PKR discount per unit multiplied by quantity
    line.lineDiscount = safeMultiply(product.discountPerUnit, quantity);

    // Payable = Subtotal + Tax - Discount
    line.linePayable = line.lineSubtotal + line.lineTax - line.lineDiscount;
    if (line.linePayable < 0) {
        line.linePayable = 0; // Prevent negative payable
    }

    return line;
}

Bill buildBill(const Order& order, const ProductCatalog& catalog, const std::string& billId) {
    Bill bill;
    bill.id = billId;
    bill.orderId = order.id;
    bill.customerName = order.customerName;
    bill.date = order.date;
    bill.time = order.time;
    bill.status = OrderStatus::Paid;
    bill.paymentMode = order.mode;
    bill.merchant = order.merchant;

    // Price every order line and aggregate grand totals
    for (const auto& orderLine : order.lines) {
        // Try to find product by ID first, then by barcode as fallback
        std::optional<Product> product = catalog.findById(orderLine.productId);
        if (!product && !orderLine.barcode.empty()) {
            product = catalog.findByBarcode(orderLine.barcode);
        }

        if (!product) {
            // Unknown product: skip gracefully without crashing
            continue;
        }

        BillLine line = priceLine(*product, orderLine.quantity);
        bill.lines.push_back(line);

        bill.subtotal += line.lineSubtotal;
        bill.tax += line.lineTax;
        bill.discount += line.lineDiscount;
        bill.payable += line.linePayable;
    }

    return bill;
}

Money changeDue(Money payable, Money tendered) {
    // Return change only if customer gave more than what is owed
    if (tendered <= payable) {
        return 0;
    }
    return tendered - payable;
}
