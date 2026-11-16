#pragma once
// OWNER: Member 2. Read-only dialog showing one product's details and barcode image.
#include <QDialog>
#include "Product.h"

class QLabel;

class ProductInfoDialog : public QDialog {
    Q_OBJECT
public:
    explicit ProductInfoDialog(const Product& product, QWidget* parent = nullptr);

private:
    void loadBarcodeImage(const QString& relativePath);

    QLabel* imageLabel_ = nullptr;
};