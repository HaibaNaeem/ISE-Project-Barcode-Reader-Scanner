#pragma once
// OWNER: Member 4.
// Read-only Customer Bill Dialog: displays detailed product lines, tax/discount breakdowns,
// grand totals, and payment status.
#include <QDialog>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include "Bill.h"

class BillDialog : public QDialog {
public:
    explicit BillDialog(const Bill& bill, QWidget* parent = nullptr);

private:
    void setupUi(const Bill& bill);
};
