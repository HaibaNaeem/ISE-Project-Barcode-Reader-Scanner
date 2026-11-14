#pragma once
// OWNER: Member 5. Lists held orders; the cashier picks one to resume.
// The dialog only shows what it is given. MainWindow then calls OrderManager::resume(selectedOrderId()).
#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QString>
#include <vector>
#include "Order.h"

class HeldOrdersDialog : public QDialog {
    Q_OBJECT
public:
    // Usage:  HeldOrdersDialog dlg(manager.heldOrders(), this);
    //         if (dlg.exec() == QDialog::Accepted) manager.resume(dlg.selectedOrderId().toStdString());
    explicit HeldOrdersDialog(const std::vector<Order>& heldOrders, QWidget* parent = nullptr);

    QString selectedOrderId() const { return selectedId_; }   // empty if nothing chosen

private slots:
    void onSelectionChanged();
    void onResumeClicked();

private:
    QListWidget* list_;
    QPushButton* resumeButton_;
    QString selectedId_;
};
