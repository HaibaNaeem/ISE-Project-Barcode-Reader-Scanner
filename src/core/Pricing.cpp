// OWNER: Member 4. STUB written by Member 5 so the project builds. Replace with the real code.
#include "Pricing.h"

BillLine priceLine(const Product&, Quantity) { return {}; }
Bill buildBill(const Order& order, const ProductCatalog&, const std::string& billId) {
    Bill b; b.id = billId; b.orderId = order.id; b.customerName = order.customerName; return b;
}
Money changeDue(Money payable, Money tendered) { return tendered > payable ? tendered - payable : 0; }
