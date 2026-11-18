#include "OrderPage.h"

#include <QtWidgets>
#include <cmath>

namespace {

QString money(Money m) {
    const bool neg = m < 0;
    if (neg) m = -m;
    return QString("%1%2.%3").arg(neg ? "-" : "").arg(m / 100).arg(m % 100, 2, 10, QChar('0'));
}

QString qtyText(Quantity q, MeasureUnit u) {
    if (u == MeasureUnit::Count) return QString::number(q / 1000);
    return QString("%1.%2").arg(q / 1000).arg(q % 1000, 3, 10, QChar('0'));
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
    nameEdit_ = new QLineEdit; nameEdit_->setPlaceholderText("Customer name");
    contactEdit_ = new QLineEdit; contactEdit_->setPlaceholderText("Contact (optional)");
    auto* top = new QHBoxLayout;
    top->addWidget(new QLabel("Customer:")); top->addWidget(nameEdit_, 2);
    top->addWidget(contactEdit_, 1);

    auto* left = new QVBoxLayout;
    auto* ph = new QLabel("Camera panel (Member 5)");
    ph->setAlignment(Qt::AlignCenter);
    ph->setFrameStyle(QFrame::Box);
    cameraHolder_ = ph;
    cameraHolder_->setMinimumSize(280, 210);
    left->addWidget(cameraHolder_);

    barcodeEdit_ = new QLineEdit; barcodeEdit_->setPlaceholderText("6-digit barcode");
    barcodeEdit_->setMaxLength(6);
    qtyBox_ = new QDoubleSpinBox;
    qtyBox_->setDecimals(3); qtyBox_->setRange(0.001, 100000); qtyBox_->setValue(1);
    auto* addBtn = new QPushButton("Add");
    left->addWidget(new QLabel("Manual barcode"));
    left->addWidget(barcodeEdit_);
    left->addWidget(new QLabel("Quantity"));
    left->addWidget(qtyBox_);
    left->addWidget(addBtn);
    left->addStretch();
    connect(barcodeEdit_, &QLineEdit::returnPressed, this, &OrderPage::onManualAdd);
    connect(addBtn, &QPushButton::clicked, this, &OrderPage::onManualAdd);

    cart_ = new QTableWidget(0, 5);
    cart_->setHorizontalHeaderLabels({"Item", "Name", "Qty", "Unit price", "Line total"});
    cart_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    cart_->setSelectionBehavior(QAbstractItemView::SelectRows);
    cart_->setSelectionMode(QAbstractItemView::SingleSelection);
    cart_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(cart_, &QTableWidget::itemSelectionChanged, this, [this] { refresh(); });

    subtotalLbl_ = new QLabel("0.00"); taxLbl_ = new QLabel("0.00");
    discountLbl_ = new QLabel("0.00"); payableLbl_ = new QLabel("0.00");
    payableLbl_->setStyleSheet("font-size:20px;font-weight:bold;");
    auto* form = new QFormLayout;
    form->addRow("Subtotal", subtotalLbl_); form->addRow("Tax", taxLbl_);
    form->addRow("Discount", discountLbl_); form->addRow("Payable", payableLbl_);

    descBtn_ = new QPushButton("Description"); plusBtn_ = new QPushButton("+ Qty");
    minusBtn_ = new QPushButton("- Qty"); removeBtn_ = new QPushButton("Remove");
    holdBtn_ = new QPushButton("Hold"); heldBtn_ = new QPushButton("Held Orders");
    payBtn_ = new QPushButton("Pay"); cancelBtn_ = new QPushButton("Cancel");
    auto* right = new QVBoxLayout;
    right->addLayout(form);
    for (auto* b : {descBtn_, plusBtn_, minusBtn_, removeBtn_, holdBtn_, heldBtn_, payBtn_, cancelBtn_})
        right->addWidget(b);
    right->addStretch();
    connect(descBtn_, &QPushButton::clicked, this, &OrderPage::onDescription);
    connect(plusBtn_, &QPushButton::clicked, this, &OrderPage::onPlus);
    connect(minusBtn_, &QPushButton::clicked, this, &OrderPage::onMinus);
    connect(removeBtn_, &QPushButton::clicked, this, &OrderPage::onRemove);
    connect(holdBtn_, &QPushButton::clicked, this, &OrderPage::onHold);
    connect(heldBtn_, &QPushButton::clicked, this, &OrderPage::onHeldOrders);
    connect(payBtn_, &QPushButton::clicked, this, &OrderPage::onPay);
    connect(cancelBtn_, &QPushButton::clicked, this, &OrderPage::onCancel);

    banner_ = new QLabel; banner_->setWordWrap(true); banner_->setMinimumHeight(28);

    auto* body = new QHBoxLayout;
    body->addLayout(left, 2); body->addWidget(cart_, 4); body->addLayout(right, 2);
    auto* root = new QVBoxLayout(this);
    root->addLayout(top); root->addLayout(body, 1); root->addWidget(banner_);
}

void OrderPage::showResult(const OperationResult& r) {
    banner_->setText(QString::fromStdString(r.message));
    banner_->setStyleSheet(r.success
        ? "background:#e6f4ea;color:#1e4620;padding:4px;"
        : "background:#fdecea;color:#7a1c1c;padding:4px;");
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
                QString::fromStdString(p->acronym), QString::fromStdString(p->name),
                qtyText(line.quantity, p->unit), money(p->unitPrice), money(lineTotal)};
            for (int c = 0; c < 5; ++c) {
                auto* it = new QTableWidgetItem(cells[c]);
                if (c >= 2) it->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                if (c == 0) it->setData(Qt::UserRole, QString::fromStdString(line.productId));
                cart_->setItem(row, c, it);
            }
            if (QString::fromStdString(line.productId) == keepId) cart_->selectRow(row);
            ++row;
        }
    }

    subtotalLbl_->setText("0.00"); taxLbl_->setText("0.00");
    discountLbl_->setText("0.00"); payableLbl_->setText("0.00");

    const bool sel = !selectedProductId().isEmpty();
    descBtn_->setEnabled(hasLines);
    plusBtn_->setEnabled(sel); minusBtn_->setEnabled(sel); removeBtn_->setEnabled(sel);
    holdBtn_->setEnabled(hasLines); payBtn_->setEnabled(hasLines);
    cancelBtn_->setEnabled(cur.has_value());
    nameEdit_->setEnabled(!cur.has_value());
    contactEdit_->setEnabled(!cur.has_value());
}

void OrderPage::addBarcode(const QString& code) {
    if (!orders_.current()) {
        auto r = orders_.startOrder(nameEdit_->text().trimmed().toStdString(),
                                    contactEdit_->text().trimmed().toStdString());
        if (!r.success) { showResult(r); return; }
    }
    const Quantity q = std::llround(qtyBox_->value() * 1000.0);
    auto r = orders_.addByBarcode(code.toStdString(), q);
    if (r.success) lastBarcode_ = code;
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
    if (!id.isEmpty()) p = catalog_.findById(id.toStdString());
    else if (!lastBarcode_.isEmpty()) p = catalog_.findByBarcode(lastBarcode_.toStdString());
    if (!p) { showResult({false, "No product to describe yet."}); return; }

    const QString text = QString::fromStdString(p->name)
        + "\nBarcode: " + QString::fromStdString(p->barcode)
        + "\nPrice: " + money(p->unitPrice);
    QMessageBox::information(this, "Product", text);
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
    showResult({false, "Held orders screen not connected yet (Member 5)."});
}

void OrderPage::onPay() {
    showResult({false, "Payment screen not connected yet (Member 4)."});
}

void OrderPage::onCancel() {
    if (QMessageBox::question(this, "Cancel order", "Cancel this order?") != QMessageBox::Yes) return;
    showResult(orders_.cancel());
    refresh();
}