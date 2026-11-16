// Tests for Member 4 Pricing module
#include "TestHelpers.h"
#include "Pricing.h"
#include "Product.h"
#include "ProductCatalog.h"
#include "Order.h"

static void testStandardMustPassCase() {
    // Must-pass check from spec:
    // price 100, tax 15, discount 10, quantity 3
    // Paisa: 10000, 1500, 1000, qty 3000
    // Expected: subtotal 30000, tax 4500, discount 3000, payable 31500
    Product p;
    p.id = "101";
    p.acronym = "ITEMA";
    p.name = "Item A";
    p.unitPrice = 10000;
    p.taxPerUnit = 1500;
    p.discountPerUnit = 1000;
    p.available = 10000;

    BillLine line = priceLine(p, 3000);
    CHECK_EQ(line.productId, std::string("101"));
    CHECK_EQ(line.acronym, std::string("ITEMA"));
    CHECK_EQ(line.unitPrice, 10000);
    CHECK_EQ(line.discountPerUnit, 1000);
    CHECK_EQ(line.quantity, 3000);
    CHECK_EQ(line.lineSubtotal, 30000);
    CHECK_EQ(line.lineTax, 4500);
    CHECK_EQ(line.lineDiscount, 3000);
    CHECK_EQ(line.linePayable, 31500);
}

static void testZeroQuantity() {
    Product p;
    p.id = "101";
    p.unitPrice = 5000;
    p.taxPerUnit = 500;
    p.discountPerUnit = 200;

    BillLine line = priceLine(p, 0);
    CHECK_EQ(line.lineSubtotal, 0);
    CHECK_EQ(line.lineTax, 0);
    CHECK_EQ(line.lineDiscount, 0);
    CHECK_EQ(line.linePayable, 0);
}

static void testWeightProduct() {
    // 1.250 kg of Sugar at 200.00 PKR/kg, tax 10.00 PKR/kg, no discount
    Product sugar;
    sugar.id = "102";
    sugar.acronym = "SUGAR";
    sugar.unit = MeasureUnit::Weight;
    sugar.unitPrice = 20000;       // 200.00 PKR
    sugar.taxPerUnit = 1000;       // 10.00 PKR
    sugar.discountPerUnit = 0;

    Quantity qty = 1250;           // 1.250 kg
    BillLine line = priceLine(sugar, qty);

    // subtotal = 20000 * 1250 / 1000 = 25000 (250.00 PKR)
    CHECK_EQ(line.lineSubtotal, 25000);
    // tax = 1000 * 1250 / 1000 = 1250 (12.50 PKR)
    CHECK_EQ(line.lineTax, 1250);
    CHECK_EQ(line.lineDiscount, 0);
    // payable = 25000 + 1250 - 0 = 26250 (262.50 PKR)
    CHECK_EQ(line.linePayable, 26250);
}

static void testRoundingHalfUp() {
    // 1.500 kg at 33.33 PKR -> 3333 paisa
    // (3333 * 1500 + 500) / 1000 = (4999500 + 500) / 1000 = 5000 paisa
    Product p;
    p.id = "103";
    p.unit = MeasureUnit::Weight;
    p.unitPrice = 3333;
    p.taxPerUnit = 0;
    p.discountPerUnit = 0;

    BillLine line = priceLine(p, 1500);
    CHECK_EQ(line.lineSubtotal, 5000);
    CHECK_EQ(line.linePayable, 5000);
}

static void testBuildBillMultipleProducts() {
    Product milk;
    milk.id = "P1";
    milk.acronym = "MILKP";
    milk.unitPrice = 45000;        // 450.00 PKR
    milk.taxPerUnit = 2000;        // 20.00 PKR
    milk.discountPerUnit = 1000;   // 10.00 PKR

    Product bread;
    bread.id = "P2";
    bread.acronym = "BREAD";
    bread.unitPrice = 12000;       // 120.00 PKR
    bread.taxPerUnit = 0;
    bread.discountPerUnit = 500;   // 5.00 PKR

    ProductCatalog catalog({milk, bread});

    Order order;
    order.id = "00000714";
    order.customerName = "Ali Khan";
    order.date = "2026-11-16";
    order.time = "14:05:09";
    order.mode = PaymentMode::Cash;
    order.lines = {
        {"P1", "", 2000}, // 2 Milk: 450*2=900, tax 20*2=40, disc 10*2=20 -> payable 920.00 (92000)
        {"P2", "", 1000}  // 1 Bread: 120*1=120, tax 0, disc 5*1=5 -> payable 115.00 (11500)
    };

    Bill bill = buildBill(order, catalog, "000000031");
    CHECK_EQ(bill.id, std::string("000000031"));
    CHECK_EQ(bill.orderId, std::string("00000714"));
    CHECK_EQ(bill.customerName, std::string("Ali Khan"));
    CHECK_EQ(bill.lines.size(), 2);

    // First line
    CHECK_EQ(bill.lines[0].lineSubtotal, 90000);
    CHECK_EQ(bill.lines[0].lineTax, 4000);
    CHECK_EQ(bill.lines[0].lineDiscount, 2000);
    CHECK_EQ(bill.lines[0].linePayable, 92000);

    // Second line
    CHECK_EQ(bill.lines[1].lineSubtotal, 12000);
    CHECK_EQ(bill.lines[1].lineTax, 0);
    CHECK_EQ(bill.lines[1].lineDiscount, 500);
    CHECK_EQ(bill.lines[1].linePayable, 11500);

    // Grand totals
    CHECK_EQ(bill.subtotal, 102000);
    CHECK_EQ(bill.tax, 4000);
    CHECK_EQ(bill.discount, 2500);
    CHECK_EQ(bill.payable, 103500);
}

static void testBuildBillUnknownProduct() {
    Product milk;
    milk.id = "P1";
    milk.unitPrice = 10000;
    ProductCatalog catalog({milk});

    Order order;
    order.id = "ORD01";
    order.lines = {
        {"P1", "", 1000},
        {"NON_EXISTENT", "", 1000} // Missing product should be gracefully skipped
    };

    Bill bill = buildBill(order, catalog, "BILL01");
    CHECK_EQ(bill.lines.size(), 1);
    CHECK_EQ(bill.payable, 10000);
}

static void testChangeDue() {
    // Exact tendered
    CHECK_EQ(changeDue(31500, 31500), 0);

    // Tendered more: 350.00 given for 315.00 bill -> 35.00 change (3500 paisa)
    CHECK_EQ(changeDue(31500, 35000), 3500);

    // Tendered less: 300.00 given for 315.00 bill -> 0 change (not enough)
    CHECK_EQ(changeDue(31500, 30000), 0);

    // Zero bill
    CHECK_EQ(changeDue(0, 5000), 5000);

    // Verify alias
    CHECK_EQ(calculateChange(10000, 15000), 5000);
}

int main() {
    testStandardMustPassCase();
    testZeroQuantity();
    testWeightProduct();
    testRoundingHalfUp();
    testBuildBillMultipleProducts();
    testBuildBillUnknownProduct();
    testChangeDue();
    return testSummary("test_pricing");
}

