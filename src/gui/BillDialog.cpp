// OWNER: Member 4.
// Read-only Customer Bill Dialog implementation.
#include "BillDialog.h"
#include "DialogStyle.h"
#include "Units.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QGroupBox>

namespace {

// Helper to format money for GUI table cells (right-aligned text)
QString guiFormatMoney(Money amount) {
    std::string s = formatMoney(amount);
    if (s != "TODO") {
        return QString::fromStdString(s + " PKR");
    }
    std::string sign = amount < 0 ? "-" : "";
    if (amount < 0) amount = -amount;
    std::string cents = std::to_string(amount % 100);
    if (cents.size() < 2) cents = "0" + cents;
    return QString::fromStdString(sign + std::to_string(amount / 100) + "." + cents + " PKR");
}

// Helper to format quantity
QString guiFormatQuantity(Quantity qty) {
    if (qty % 1000 == 0) {
        return QString::number(qty / 1000);
    }
    std::string q = std::to_string(qty % 1000);
    while (q.size() < 3) q = "0" + q;
    return QString::fromStdString(std::to_string(qty / 1000) + "." + q);
}

} // anonymous namespace

BillDialog::BillDialog(const Bill& bill, QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Bill Summary - #" + QString::fromStdString(bill.id));
    setMinimumSize(780, 540);
    setStyleSheet(DialogStyle::dialogStyleSheet());

    setupUi(bill);
}

void BillDialog::setupUi(const Bill& bill) {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    // 1. Header Information Card (Bill ID, Order ID, Customer, Date/Time, Status Badge)
    auto* headerBox = new QGroupBox("Invoice Details", this);
    auto* headerLayout = new QGridLayout(headerBox);
    headerLayout->setContentsMargins(12, 12, 12, 12);
    headerLayout->setSpacing(8);

    auto* billIdLabel = new QLabel(QString("<b>Bill ID:</b> %1").arg(QString::fromStdString(bill.id)), headerBox);
    auto* orderIdLabel = new QLabel(QString("<b>Order ID:</b> %1").arg(QString::fromStdString(bill.orderId)), headerBox);
    auto* custLabel = new QLabel(QString("<b>Customer:</b> %1").arg(QString::fromStdString(bill.customerName)), headerBox);

    QString dateTimeStr = QString::fromStdString(bill.date + " " + bill.time).trimmed();
    if (dateTimeStr.isEmpty()) dateTimeStr = "N/A";
    auto* dateLabel = new QLabel(QString("<b>Date/Time:</b> %1").arg(dateTimeStr), headerBox);

    // Status Badge
    QString statusStr = QString::fromStdString(orderStatusToText(bill.status)).toUpper();
    auto* statusBadge = new QLabel(statusStr, headerBox);
    statusBadge->setAlignment(Qt::AlignCenter);
    QString badgeColor = (bill.status == OrderStatus::Paid) ? DialogStyle::kSuccess : DialogStyle::kMutedText;
    statusBadge->setStyleSheet(QString(
        "background-color: %1; color: #FFFFFF; font-weight: bold; border-radius: 4px; padding: 4px 12px;"
    ).arg(badgeColor));

    headerLayout->addWidget(billIdLabel, 0, 0);
    headerLayout->addWidget(orderIdLabel, 0, 1);
    headerLayout->addWidget(statusBadge, 0, 2, Qt::AlignRight);
    headerLayout->addWidget(custLabel, 1, 0);
    headerLayout->addWidget(dateLabel, 1, 1);

    mainLayout->addWidget(headerBox);

    // 2. Product Table (One row per product on the order)
    auto* table = new QTableWidget(this);
    table->setColumnCount(8);
    table->setHorizontalHeaderLabels({
        "Product", "Unit Price", "Disc / Unit", "Qty",
        "Subtotal", "Tax", "Discount", "Total Payable"
    });

    table->setRowCount(static_cast<int>(bill.lines.size()));
    table->setAlternatingRowColors(true);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->verticalHeader()->setVisible(false);

    for (int i = 0; i < static_cast<int>(bill.lines.size()); ++i) {
        const auto& line = bill.lines[i];

        auto* itemAcronym = new QTableWidgetItem(QString::fromStdString(line.acronym));
        auto* itemPrice = new QTableWidgetItem(guiFormatMoney(line.unitPrice));
        auto* itemDiscUnit = new QTableWidgetItem(guiFormatMoney(line.discountPerUnit));
        auto* itemQty = new QTableWidgetItem(guiFormatQuantity(line.quantity));
        auto* itemSubtotal = new QTableWidgetItem(guiFormatMoney(line.lineSubtotal));
        auto* itemTax = new QTableWidgetItem(guiFormatMoney(line.lineTax));
        auto* itemDiscount = new QTableWidgetItem(guiFormatMoney(line.lineDiscount));
        auto* itemPayable = new QTableWidgetItem(guiFormatMoney(line.linePayable));

        // Right-align numbers and money columns
        itemPrice->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemDiscUnit->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemQty->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemSubtotal->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemTax->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemDiscount->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemPayable->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        // Highlight line payable
        itemPayable->setFont(QFont(font().family(), font().pointSize(), QFont::Bold));

        table->setItem(i, 0, itemAcronym);
        table->setItem(i, 1, itemPrice);
        table->setItem(i, 2, itemDiscUnit);
        table->setItem(i, 3, itemQty);
        table->setItem(i, 4, itemSubtotal);
        table->setItem(i, 5, itemTax);
        table->setItem(i, 6, itemDiscount);
        table->setItem(i, 7, itemPayable);
    }

    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    for (int c = 1; c < 8; ++c) {
        table->horizontalHeader()->setSectionResizeMode(c, QHeaderView::ResizeToContents);
    }

    mainLayout->addWidget(table, 1);

    // 3. Totals and Summary Card (Bottom)
    auto* bottomLayout = new QHBoxLayout();

    // Mode details (Left)
    QString modeName = QString::fromStdString(paymentModeToText(bill.paymentMode)).toUpper();
    QString paymentDetails = "<b>Payment Mode:</b> " + modeName;
    if (!bill.merchant.empty()) {
        paymentDetails += " (" + QString::fromStdString(bill.merchant) + ")";
    }
    auto* payModeLabel = new QLabel(paymentDetails, this);
    bottomLayout->addWidget(payModeLabel, 0, Qt::AlignBottom);

    bottomLayout->addStretch(1);

    // Grand totals summary grid (Right)
    auto* totalsBox = new QGroupBox("Payment Summary", this);
    auto* totalsGrid = new QGridLayout(totalsBox);
    totalsGrid->setContentsMargins(12, 10, 12, 10);
    totalsGrid->setSpacing(6);

    totalsGrid->addWidget(new QLabel("Subtotal (excl. Tax):", totalsBox), 0, 0);
    auto* subtotalVal = new QLabel(guiFormatMoney(bill.subtotal), totalsBox);
    subtotalVal->setAlignment(Qt::AlignRight);
    totalsGrid->addWidget(subtotalVal, 0, 1);

    totalsGrid->addWidget(new QLabel("Total Tax:", totalsBox), 1, 0);
    auto* taxVal = new QLabel(guiFormatMoney(bill.tax), totalsBox);
    taxVal->setAlignment(Qt::AlignRight);
    totalsGrid->addWidget(taxVal, 1, 1);

    totalsGrid->addWidget(new QLabel("Total Discount:", totalsBox), 2, 0);
    auto* discVal = new QLabel(guiFormatMoney(bill.discount), totalsBox);
    discVal->setAlignment(Qt::AlignRight);
    totalsGrid->addWidget(discVal, 2, 1);

    auto* payableTitle = new QLabel("Grand Total Payable:", totalsBox);
    payableTitle->setStyleSheet("font-size: 13pt; font-weight: bold;");
    totalsGrid->addWidget(payableTitle, 3, 0);

    auto* payableVal = new QLabel(guiFormatMoney(bill.payable), totalsBox);
    payableVal->setAlignment(Qt::AlignRight);
    payableVal->setStyleSheet(QString("font-size: 16pt; font-weight: bold; color: %1;")
                             .arg(DialogStyle::kPrimary));
    totalsGrid->addWidget(payableVal, 3, 1);

    bottomLayout->addWidget(totalsBox);
    mainLayout->addLayout(bottomLayout);

    // 4. Close Button
    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch(1);

    auto* closeBtn = new QPushButton("Close", this);
    closeBtn->setObjectName("closeBtn");
    closeBtn->setDefault(true);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    btnLayout->addWidget(closeBtn);
    mainLayout->addLayout(btnLayout);
}

