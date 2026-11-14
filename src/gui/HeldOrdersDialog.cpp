#include "HeldOrdersDialog.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

HeldOrdersDialog::HeldOrdersDialog(const std::vector<Order>& heldOrders, QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("Held Orders"));
    setMinimumWidth(460);

    list_ = new QListWidget(this);
    for (const Order& o : heldOrders) {
        const QString customer = o.customerName.empty() ? tr("(no name)") : QString::fromStdString(o.customerName);
        const QString text = tr("Order %1  |  %2  |  %3 %4  |  %5 item(s)")
                                 .arg(QString::fromStdString(o.id), customer,
                                      QString::fromStdString(o.date), QString::fromStdString(o.time))
                                 .arg(static_cast<qulonglong>(o.lines.size()));
        auto* item = new QListWidgetItem(text, list_);
        item->setData(Qt::UserRole, QString::fromStdString(o.id));
    }

    resumeButton_ = new QPushButton(tr("Resume"), this);
    auto* closeButton = new QPushButton(tr("Close"), this);
    resumeButton_->setEnabled(false);
    resumeButton_->setDefault(true);

    auto* buttons = new QHBoxLayout;
    buttons->addStretch(1);
    buttons->addWidget(resumeButton_);
    buttons->addWidget(closeButton);

    auto* layout = new QVBoxLayout(this);
    if (heldOrders.empty()) layout->addWidget(new QLabel(tr("There are no held orders."), this));
    layout->addWidget(list_);
    layout->addLayout(buttons);

    connect(list_, &QListWidget::itemSelectionChanged, this, &HeldOrdersDialog::onSelectionChanged);
    connect(list_, &QListWidget::itemDoubleClicked, this, [this] { onResumeClicked(); });
    connect(resumeButton_, &QPushButton::clicked, this, &HeldOrdersDialog::onResumeClicked);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);
}

void HeldOrdersDialog::onSelectionChanged() {
    resumeButton_->setEnabled(list_->currentItem() != nullptr && !list_->selectedItems().isEmpty());
}

void HeldOrdersDialog::onResumeClicked() {
    QListWidgetItem* item = list_->currentItem();
    if (!item) return;
    selectedId_ = item->data(Qt::UserRole).toString();
    accept();
}
