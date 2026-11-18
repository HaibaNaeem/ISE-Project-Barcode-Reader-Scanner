// OWNER: Member 1. Order screen layout and cashier interactions.
#include "OrderPage.h"
#include "CameraPanel.h"
#include "ProductInfoDialog.h"
#include "HeldOrdersDialog.h"
#include "PaymentDialog.h"
#include "BillDialog.h"
#include "Pricing.h"
#include "Units.h"

#include <QtWidgets>
#include <cmath>

namespace {

QString moneyText(Money m) {
    return QString::fromStdString(formatMoney(m)) + " PKR";
}

QString qtyText(Quantity q, MeasureUnit u) {
    return QString::fromStdString(formatQuantity(q, u));
}

} // namespace

OrderPage::OrderPage(ProductCatalog& catalog, OrderManager& orders,
                     CheckoutService& checkout, QWidget* parent)
    : QWidget(parent), catalog_(catalog), orders_(orders), checkout_(checkout) {
    buildUi();
    refresh();
}

OrderPage::~OrderPage() = default;

void OrderPage::buildUi() {
    nameEdit_ = new QLineEdit;
    nameEdit_->setPlaceholderText("Customer name");
    contactEdit_ = new QLineEdit;
    contactEdit_->setPlaceholderText("Contact number (optional)");

    auto* topLayout = new QHBoxLayout;
    topLayout->addWidget(new QLabel("Customer:"));
    topLayout->addWidget(nameEdit_, 2);
    topLayout->addWidget(contactEdit_, 1);

    // Left Panel: Camera + Manual Barcode + Quantity Box
    auto* leftLayout = new QVBoxLayout;
    cameraPanel_ = new CameraPanel(this);
    cameraPanel_->setMinimumSize(320, 240);
    connect(cameraPanel_, &CameraPanel::barcodeScanned, this, &OrderPage::addBarcode);
    connect(cameraPanel_, &CameraPanel::cameraMessage, this, [this](const QString& msg) {
        showResult({false, msg.toStdString()});
    });
    // Start camera once the event loop is active so DirectShow COM doesn't deadlock the GUI thread during construction
    QTimer::singleShot(100, cameraPanel_, &CameraPanel::start);

    leftLayout->addWidget(cameraPanel_);

    barcodeEdit_ = new QLineEdit;
    barcodeEdit_->setPlaceholderText("6-digit barcode");
    barcodeEdit_->setMaxLength(6);

    qtyBox_ = new QDoubleSpinBox;
    qtyBox_->setDecimals(3);
    qtyBox_->setRange(0.001, 100000.0);
    qtyBox_->setValue(1.0);

    auto* addBtn = new QPushButton("Add Item");
    addBtn->setStyleSheet("padding: 6px; font-weight: bold;");

    leftLayout->addWidget(new QLabel("Manual Barcode:"));
    leftLayout->addWidget(barcodeEdit_);
    leftLayout->addWidget(new QLabel("Quantity:"));
    leftLayout->addWidget(qtyBox_);
    leftLayout->addWidget(addBtn);
    leftLayout->addStretch();

    connect(barcodeEdit_, &QLineEdit::returnPressed, this, &OrderPage::onManualAdd);
    connect(addBtn, &QPushButton::clicked, this, &OrderPage::onManualAdd);

    // Middle Panel: Cart Table
    cart_ = new QTableWidget(0, 5);
    cart_->setHorizontalHeaderLabels({"Item", "Name", "Qty", "Unit Price", "Line Total"});
    cart_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    cart_->setSelectionBehavior(QAbstractItemView::SelectRows);
    cart_->setSelectionMode(QAbstractItemView::SingleSelection);
    cart_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(cart_, &QTableWidget::itemSelectionChanged, this, [this] { refresh(); });

    // Right Panel: Financial Totals + Action Buttons
    subtotalLbl_ = new QLabel("0.00 PKR");
    taxLbl_ = new QLabel("0.00 PKR");
    discountLbl_ = new QLabel("0.00 PKR");
    payableLbl_ = new QLabel("0.00 PKR");
    payableLbl_->setStyleSheet("font-size: 22px; font-weight: bold; color: #1e3a8a;");

    auto* formLayout = new QFormLayout;
    formLayout->addRow("Subtotal:", subtotalLbl_);
    formLayout->addRow("Tax:", taxLbl_);
    formLayout->addRow("Discount:", discountLbl_);
    formLayout->addRow("Payable:", payableLbl_);

    descBtn_ = new QPushButton("Description");
    plusBtn_ = new QPushButton("+ Qty");
    minusBtn_ = new QPushButton("- Qty");
    removeBtn_ = new QPushButton("Remove");
    holdBtn_ = new QPushButton("Hold Order");
    heldBtn_ = new QPushButton("Held Orders");
    payBtn_ = new QPushButton("Pay Order");
    payBtn_->setStyleSheet("background-color: #15803d; color: white; font-weight: bold; padding: 8px;");
    cancelBtn_ = new QPushButton("Cancel Order");
    cancelBtn_->setStyleSheet("background-color: #b91c1c; color: white; padding: 6px;");

    auto* rightLayout = new QVBoxLayout;
    rightLayout->addLayout(formLayout);
    for (auto* btn : {descBtn_, plusBtn_, minusBtn_, removeBtn_, holdBtn_, heldBtn_, payBtn_, cancelBtn_}) {
        rightLayout->addWidget(btn);
    }
    rightLayout->addStretch();

    connect(descBtn_, &QPushButton::clicked, this, &OrderPage::onDescription);
    connect(plusBtn_, &QPushButton::clicked, this, &OrderPage::onPlus);
    connect(minusBtn_, &QPushButton::clicked, this, &OrderPage::onMinus);
    connect(removeBtn_, &QPushButton::clicked, this, &OrderPage::onRemove);
    connect(holdBtn_, &QPushButton::clicked, this, &OrderPage::onHold);
    connect(heldBtn_, &QPushButton::clicked, this, &OrderPage::onHeldOrders);
    connect(payBtn_, &QPushButton::clicked, this, &OrderPage::onPay);
    connect(cancelBtn_, &QPushButton::clicked, this, &OrderPage::onCancel);

    // Status Banner at bottom
    banner_ = new QLabel;
    banner_->setWordWrap(true);
    banner_->setMinimumHeight(32);
    banner_->setStyleSheet("background:#f3f4f6; color:#374151; padding: 6px; font-weight: bold;");

    auto* bodyLayout = new QHBoxLayout;
    bodyLayout->addLayout(leftLayout, 3);
    bodyLayout->addWidget(cart_, 5);
    bodyLayout->addLayout(rightLayout, 2);

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->addLayout(topLayout);
    rootLayout->addLayout(bodyLayout, 1);
    rootLayout->addWidget(banner_);
}

void OrderPage::showResult(const OperationResult& r) {
    banner_->setText(QString::fromStdString(r.message));
    banner_->setStyleSheet(r.success
        ? "background:#e6f4ea; color:#1e4620; padding:6px; font-weight:bold; border: 1px solid #a3cfbb;"
        : "background:#fdecea; color:#7a1c1c; padding:6px; font-weight:bold; border: 1px solid #f1aeb5;");
}

QString OrderPage::selectedProductId() const {
    const int row = cart_->currentRow();
    if (row < 0 || !cart_->item(row, 0)) return {};
    return cart_->item(row, 0)->data(Qt::UserRole).toString();
}

void OrderPage::refresh() {
    const QString keepId = selectedProductId();
    const auto& cur = orders_.current();
    const bool hasLines = cur && !cur->lines.empty();

    QSignalBlocker block(cart_);
    cart_->setRowCount(0);

    if (hasLines) {
        int row = 0;
        for (const auto& line : cur->lines) {
            const auto p = catalog_.findById(line.productId);
            if (!p) continue;
            cart_->insertRow(row);
            const Money lineTotal = multiplyPriceByQuantity(p->unitPrice, line.quantity);
            const QStringList cells = {
                QString::fromStdString(p->acronym),
                QString::fromStdString(p->name),
                qtyText(line.quantity, p->unit),
                moneyText(p->unitPrice),
                moneyText(lineTotal)
            };
            for (int c = 0; c < 5; ++c) {
                auto* it = new QTableWidgetItem(cells[c]);
                if (c >= 2) it->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                if (c == 0) it->setData(Qt::UserRole, QString::fromStdString(line.productId));
                cart_->setItem(row, c, it);
            }
            if (QString::fromStdString(line.productId) == keepId) cart_->selectRow(row);
            ++row;
        }

        // Totals come strictly from buildBill()
        Bill draft = buildBill(*cur, catalog_, "");
        subtotalLbl_->setText(moneyText(draft.subtotal));
        taxLbl_->setText(moneyText(draft.tax));
        discountLbl_->setText(moneyText(draft.discount));
        payableLbl_->setText(moneyText(draft.payable));
    } else {
        subtotalLbl_->setText("0.00 PKR");
        taxLbl_->setText("0.00 PKR");
        discountLbl_->setText("0.00 PKR");
        payableLbl_->setText("0.00 PKR");
    }

    const bool sel = !selectedProductId().isEmpty();
    descBtn_->setEnabled(hasLines);
    plusBtn_->setEnabled(sel);
    minusBtn_->setEnabled(sel);
    removeBtn_->setEnabled(sel);
    holdBtn_->setEnabled(hasLines);
    payBtn_->setEnabled(hasLines);
    cancelBtn_->setEnabled(cur.has_value());
    nameEdit_->setEnabled(!cur.has_value());
    contactEdit_->setEnabled(!cur.has_value());
}

void OrderPage::addBarcode(const QString& code) {
    if (!orders_.current().has_value()) {
        auto r = orders_.startOrder(nameEdit_->text().trimmed().toStdString(),
                                    contactEdit_->text().trimmed().toStdString());
        if (!r.success) {
            showResult(r);
            return;
        }
    }
    const Quantity q = std::llround(qtyBox_->value() * 1000.0);
    auto r = orders_.addByBarcode(code.toStdString(), q);
    if (r.success) {
        lastBarcode_ = code;
    }
    showResult(r);
    refresh();
}

void OrderPage::onManualAdd() {
    const QString code = barcodeEdit_->text().trimmed();
    if (code.isEmpty()) return;
    addBarcode(code);
    barcodeEdit_->clear();
    barcodeEdit_->setFocus();
}

void OrderPage::onDescription() {
    std::optional<Product> p;
    const QString id = selectedProductId();
    if (!id.isEmpty()) {
        p = catalog_.findById(id.toStdString());
    } else if (!lastBarcode_.isEmpty()) {
        p = catalog_.findByBarcode(lastBarcode_.toStdString());
    }

    if (!p) {
        showResult({false, "No product selected to describe."});
        return;
    }

    ProductInfoDialog dlg(*p, this);
    dlg.exec();
}

void OrderPage::changeSelectedQuantity(int direction) {
    const QString id = selectedProductId();
    const auto& cur = orders_.current();
    if (id.isEmpty() || !cur) return;
    const auto p = catalog_.findById(id.toStdString());
    if (!p) return;

    const Quantity step = (p->unit == MeasureUnit::Weight) ? 100 : 1000;
    for (const auto& line : cur->lines) {
        if (QString::fromStdString(line.productId) != id) continue;
        const Quantity next = line.quantity + direction * step;
        auto r = (next <= 0) ? orders_.removeLine(id.toStdString())
                             : orders_.setQuantity(id.toStdString(), next);
        showResult(r);
        break;
    }
    refresh();
}

void OrderPage::onPlus()  { changeSelectedQuantity(+1); }
void OrderPage::onMinus() { changeSelectedQuantity(-1); }

void OrderPage::onRemove() {
    const QString id = selectedProductId();
    if (id.isEmpty()) return;
    showResult(orders_.removeLine(id.toStdString()));
    refresh();
}

void OrderPage::onHold() {
    showResult(orders_.hold());
    refresh();
}

void OrderPage::onHeldOrders() {
    HeldOrdersDialog dlg(orders_.heldOrders(), this);
    if (dlg.exec() == QDialog::Accepted) {
        QString oid = dlg.selectedOrderId();
        if (!oid.isEmpty()) {
            auto r = orders_.resume(oid.toStdString());
            showResult(r);
            refresh();
        }
    }
}

void OrderPage::onPay() {
    const auto& cur = orders_.current();
    if (!cur || cur->lines.empty()) {
        showResult({false, "No active order to pay."});
        return;
    }

    Bill draft = buildBill(*cur, catalog_, "");
    PaymentDialog dlg(draft.payable, this);
    if (dlg.exec() == QDialog::Accepted) {
        PaymentInfo payment = dlg.paymentInfo();
        Bill finalBill;
        auto r = checkout_.pay(orders_, payment, finalBill);
        showResult(r);
        if (r.success) {
            BillDialog billDlg(finalBill, this);
            billDlg.exec();
        }
        refresh();
    }
}

void OrderPage::onCancel() {
    if (QMessageBox::question(this, "Cancel order", "Are you sure you want to cancel this order?") != QMessageBox::Yes) {
        return;
    }
    showResult(orders_.cancel());
    refresh();
}
