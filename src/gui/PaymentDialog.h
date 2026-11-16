#pragma once
// OWNER: Member 4.
// Cashier Payment Dialog: select payment mode (Cash, Card, Wallet), enter merchant or tendered amount,
// display calculated change, and validate before confirming payment.
#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QRadioButton>
#include <QPushButton>
#include <QButtonGroup>
#include "Bill.h"
#include "Units.h"

class PaymentDialog : public QDialog {
public:
    explicit PaymentDialog(Money payableAmount, QWidget* parent = nullptr);

    // Get the validated payment information chosen by cashier
    PaymentInfo paymentInfo() const;

    // Get the cash tendered amount (in paisa; 0 if non-cash)
    Money tenderedAmount() const;

    // Get the calculated change amount (in paisa; 0 if non-cash)
    Money changeAmount() const;

private:
    void onPaymentModeChanged(int id);
    void onTenderedTextChanged(const QString& text);
    void onConfirmClicked();

    void setupUi();
    void updateModeFields();
    void updateChangeCalculation();

    Money payableAmount_ = 0;
    Money tenderedAmount_ = 0;
    Money changeAmount_ = 0;

    // UI elements
    QLabel* payableLabel_ = nullptr;
    QButtonGroup* modeGroup_ = nullptr;
    QRadioButton* cashRadio_ = nullptr;
    QRadioButton* cardRadio_ = nullptr;
    QRadioButton* walletRadio_ = nullptr;

    QWidget* merchantContainer_ = nullptr;
    QLineEdit* merchantEdit_ = nullptr;

    QWidget* cashContainer_ = nullptr;
    QLineEdit* tenderedEdit_ = nullptr;
    QLabel* changeLabel_ = nullptr;

    QLabel* errorLabel_ = nullptr;
    QPushButton* confirmBtn_ = nullptr;
    QPushButton* cancelBtn_ = nullptr;
};
