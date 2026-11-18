// OWNER: Member 4.
// Cashier Payment Dialog implementation.
#include "PaymentDialog.h"
#include "DialogStyle.h"
#include "Pricing.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>

namespace {

// Helper to format money for GUI display without doing any money math
QString guiFormatMoney(Money amount) {
    std::string s = formatMoney(amount);
    if (s != "TODO") {
        return QString::fromStdString(s + " PKR");
    }
    // Fallback if M2 stub is active
    std::string sign = amount < 0 ? "-" : "";
    if (amount < 0) amount = -amount;
    std::string cents = std::to_string(amount % 100);
    if (cents.size() < 2) cents = "0" + cents;
    return QString::fromStdString(sign + std::to_string(amount / 100) + "." + cents + " PKR");
}

// Helper to parse money input from QLineEdit
bool guiParseMoney(const QString& text, Money& out) {
    std::string str = text.trimmed().toStdString();
    OperationResult r = parseMoney(str, out);
    if (r.success) {
        return true;
    }
    // Fallback if M2 stub is active
    try {
        size_t dot = str.find('.');
        if (dot == std::string::npos) {
            out = std::stoll(str) * 100;
            return true;
        }
        std::string whole = str.substr(0, dot);
        std::string frac = str.substr(dot + 1);
        if (frac.size() == 1) frac += "0";
        else if (frac.size() > 2) frac = frac.substr(0, 2);
        out = std::stoll(whole) * 100 + std::stoll(frac);
        return true;
    } catch (...) {
        return false;
    }
}

} // anonymous namespace

PaymentDialog::PaymentDialog(Money payableAmount, QWidget* parent)
    : QDialog(parent), payableAmount_(payableAmount) {
    setWindowTitle("Checkout - Payment");
    setMinimumWidth(440);
    setStyleSheet(DialogStyle::dialogStyleSheet());

    setupUi();
    updateModeFields();
}

void PaymentDialog::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(14);

    // 1. Total Payable banner (Large bold text)
    auto* payableBox = new QGroupBox("Total Amount Due", this);
    auto* payableLayout = new QVBoxLayout(payableBox);
    payableLabel_ = new QLabel(guiFormatMoney(payableAmount_), payableBox);
    payableLabel_->setAlignment(Qt::AlignCenter);
    payableLabel_->setStyleSheet(QString("font-size: 20pt; font-weight: bold; color: %1; padding: 6px;")
                                .arg(DialogStyle::kPrimary));
    payableLayout->addWidget(payableLabel_);
    mainLayout->addWidget(payableBox);

    // 2. Payment Mode selection (Radio buttons)
    auto* modeBox = new QGroupBox("Select Payment Mode", this);
    auto* modeLayout = new QHBoxLayout(modeBox);
    modeGroup_ = new QButtonGroup(this);

    cashRadio_ = new QRadioButton("Cash", modeBox);
    cardRadio_ = new QRadioButton("Card", modeBox);
    walletRadio_ = new QRadioButton("Digital Wallet", modeBox);

    cashRadio_->setChecked(true);
    modeGroup_->addButton(cashRadio_, static_cast<int>(PaymentMode::Cash));
    modeGroup_->addButton(cardRadio_, static_cast<int>(PaymentMode::Card));
    modeGroup_->addButton(walletRadio_, static_cast<int>(PaymentMode::Wallet));

    modeLayout->addWidget(cashRadio_);
    modeLayout->addWidget(cardRadio_);
    modeLayout->addWidget(walletRadio_);
    mainLayout->addWidget(modeBox);

    connect(modeGroup_, &QButtonGroup::idClicked, this, [this](int id) { onPaymentModeChanged(id); });

    // 3. Cash details container (Tendered amount + Change Due)
    cashContainer_ = new QWidget(this);
    auto* cashForm = new QFormLayout(cashContainer_);
    cashForm->setContentsMargins(0, 0, 0, 0);
    cashForm->setSpacing(10);

    tenderedEdit_ = new QLineEdit(cashContainer_);
    tenderedEdit_->setPlaceholderText("Enter amount received in PKR (e.g. 500.00)");
    connect(tenderedEdit_, &QLineEdit::textChanged, this, [this](const QString& text) { onTenderedTextChanged(text); });

    changeLabel_ = new QLabel("0.00 PKR", cashContainer_);
    changeLabel_->setStyleSheet(QString("font-size: 13pt; font-weight: bold; color: %1;")
                                .arg(DialogStyle::kSuccess));

    cashForm->addRow("Amount Received:", tenderedEdit_);
    cashForm->addRow("Change Due:", changeLabel_);
    mainLayout->addWidget(cashContainer_);

    // 4. Merchant Name container (for Card / Wallet)
    merchantContainer_ = new QWidget(this);
    auto* merchantForm = new QFormLayout(merchantContainer_);
    merchantForm->setContentsMargins(0, 0, 0, 0);
    merchantForm->setSpacing(10);

    merchantEdit_ = new QLineEdit(merchantContainer_);
    merchantEdit_->setPlaceholderText("e.g. Meezan Bank, EasyPaisa, JazzCash");
    merchantForm->addRow("Merchant Name:", merchantEdit_);
    mainLayout->addWidget(merchantContainer_);

    // 5. Validation error display
    errorLabel_ = new QLabel(this);
    errorLabel_->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 10pt;").arg(DialogStyle::kDanger));
    errorLabel_->setWordWrap(true);
    errorLabel_->hide();
    mainLayout->addWidget(errorLabel_);

    // 6. Action buttons (Confirm Payment & Cancel)
    auto* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);

    cancelBtn_ = new QPushButton("Cancel", this);
    cancelBtn_->setObjectName("cancelBtn");
    connect(cancelBtn_, &QPushButton::clicked, this, &QDialog::reject);

    confirmBtn_ = new QPushButton("Confirm Payment", this);
    confirmBtn_->setObjectName("confirmBtn");
    confirmBtn_->setDefault(true);
    connect(confirmBtn_, &QPushButton::clicked, this, [this]() { onConfirmClicked(); });

    btnLayout->addWidget(cancelBtn_);
    btnLayout->addWidget(confirmBtn_);
    mainLayout->addLayout(btnLayout);

    // Tab ordering
    setTabOrder(cashRadio_, cardRadio_);
    setTabOrder(cardRadio_, walletRadio_);
    setTabOrder(walletRadio_, tenderedEdit_);
    setTabOrder(tenderedEdit_, merchantEdit_);
    setTabOrder(merchantEdit_, confirmBtn_);
    setTabOrder(confirmBtn_, cancelBtn_);
}

void PaymentDialog::onPaymentModeChanged(int) {
    errorLabel_->hide();
    updateModeFields();
}

void PaymentDialog::updateModeFields() {
    PaymentMode mode = static_cast<PaymentMode>(modeGroup_->checkedId());
    if (mode == PaymentMode::Cash) {
        cashContainer_->show();
        merchantContainer_->hide();
        merchantEdit_->clear();
        updateChangeCalculation();
        tenderedEdit_->setFocus();
    } else {
        cashContainer_->hide();
        merchantContainer_->show();
        merchantEdit_->setFocus();
    }
}

void PaymentDialog::onTenderedTextChanged(const QString&) {
    errorLabel_->hide();
    updateChangeCalculation();
}

void PaymentDialog::updateChangeCalculation() {
    Money tendered = 0;
    if (guiParseMoney(tenderedEdit_->text(), tendered)) {
        tenderedAmount_ = tendered;
        // Delegate cash change math to Pricing service (GUI does NO financial math)
        changeAmount_ = changeDue(payableAmount_, tendered);
        changeLabel_->setText(guiFormatMoney(changeAmount_));
    } else {
        tenderedAmount_ = 0;
        changeAmount_ = 0;
        changeLabel_->setText("0.00 PKR");
    }
}

void PaymentDialog::onConfirmClicked() {
    errorLabel_->hide();
    PaymentMode mode = static_cast<PaymentMode>(modeGroup_->checkedId());

    if (mode == PaymentMode::Card || mode == PaymentMode::Wallet) {
        QString merchant = merchantEdit_->text().trimmed();
        if (merchant.isEmpty()) {
            errorLabel_->setText("Merchant name is required for card/wallet payment.");
            errorLabel_->show();
            merchantEdit_->setFocus();
            return;
        }
    } else if (mode == PaymentMode::Cash) {
        Money tendered = 0;
        if (!guiParseMoney(tenderedEdit_->text(), tendered)) {
            errorLabel_->setText("Please enter a valid cash amount.");
            errorLabel_->show();
            tenderedEdit_->setFocus();
            return;
        }
        if (tendered < payableAmount_) {
            errorLabel_->setText("Tendered amount must be equal to or greater than the amount due.");
            errorLabel_->show();
            tenderedEdit_->setFocus();
            return;
        }
        tenderedAmount_ = tendered;
        changeAmount_ = changeDue(payableAmount_, tendered);
    }

    accept();
}

PaymentInfo PaymentDialog::paymentInfo() const {
    PaymentInfo info;
    info.mode = static_cast<PaymentMode>(modeGroup_->checkedId());
    if (info.mode == PaymentMode::Card || info.mode == PaymentMode::Wallet) {
        info.merchant = merchantEdit_->text().trimmed().toStdString();
    } else {
        info.merchant.clear();
    }
    return info;
}

Money PaymentDialog::tenderedAmount() const {
    return tenderedAmount_;
}

Money PaymentDialog::changeAmount() const {
    return changeAmount_;
}

