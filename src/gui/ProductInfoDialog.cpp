#include "ProductInfoDialog.h"
#include "Units.h"

#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QFormLayout>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

namespace {
QString money(Money m) { return QString::fromStdString(formatMoney(m)) + " PKR"; }

QLabel* valueLabel(const QString& text)
{
    auto* l = new QLabel(text);
    l->setTextInteractionFlags(Qt::TextSelectableByMouse);
    return l;
}
}

ProductInfoDialog::ProductInfoDialog(const Product& p, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Product Information");
    setModal(true);

    auto* layout = new QVBoxLayout(this);

    imageLabel_ = new QLabel;
    imageLabel_->setAlignment(Qt::AlignCenter);
    imageLabel_->setMinimumSize(320, 120);
    layout->addWidget(imageLabel_);
    loadBarcodeImage(QString::fromStdString(p.barcodeImagePath));

    const QString unitText = QString::fromStdString(measureUnitToText(p.unit));
    const QString stockText = QString::fromStdString(formatQuantity(p.available, p.unit))
                              + (p.unit == MeasureUnit::Weight ? " kg" : " pcs");

    auto* form = new QFormLayout;
    form->addRow("Name:",            valueLabel(QString::fromStdString(p.name)));
    form->addRow("Acronym:",         valueLabel(QString::fromStdString(p.acronym)));
    form->addRow("Product ID:",      valueLabel(QString::fromStdString(p.id)));
    form->addRow("Barcode:",         valueLabel(QString::fromStdString(p.barcode)));
    form->addRow("Unit:",            valueLabel(unitText));
    form->addRow("Unit price:",      valueLabel(money(p.unitPrice)));
    form->addRow("Tax / unit:",      valueLabel(money(p.taxPerUnit)));
    form->addRow("Discount / unit:", valueLabel(money(p.discountPerUnit)));
    form->addRow("In stock:",        valueLabel(stockText));
    layout->addLayout(form);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

void ProductInfoDialog::loadBarcodeImage(const QString& relativePath)
{
    if (relativePath.isEmpty()) {
        imageLabel_->setText("image not found");
        return;
    }

    // Path is relative to the folder of the .exe; fall back to the current folder.
    const QString fromExe = QCoreApplication::applicationDirPath() + "/" + relativePath;
    const QString path = QFileInfo::exists(fromExe) ? fromExe : relativePath;

    QPixmap pix(path);
    if (pix.isNull()) {
        imageLabel_->setText("image not found");
        return;
    }
    imageLabel_->setPixmap(pix.scaledToWidth(320, Qt::FastTransformation));
}