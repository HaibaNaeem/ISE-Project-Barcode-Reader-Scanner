#pragma once
#include <QWidget>
#include <QString>
#include <string>
#include "core/ProductCatalog.h"
#include "core/OrderManager.h"
#include "core/CheckoutService.h"
#include "services/CommonTypes.h"

class QLabel;
class QLineEdit;
class QDoubleSpinBox;
class QTableWidget;
class QPushButton;

// The order screen. Never reads files, never does money maths:
// every number shown comes from buildBill().
class OrderPage : public QWidget {
    Q_OBJECT
public:
    OrderPage(ProductCatalog& catalog, OrderManager& orders,
              CheckoutService& checkout, QWidget* parent = nullptr);
    ~OrderPage() override;

    void showResult(const OperationResult& r);   // status banner

public slots:
    void addBarcode(const QString& code);        // camera or manual box

private slots:
    void onManualAdd();
    void onDescription();
    void onPlus();
    void onMinus();
    void onRemove();
    void onHold();
    void onHeldOrders();
    void onPay();
    void onCancel();

private:
    void buildUi();
    void refresh();
    QString selectedProductId() const;
    void changeSelectedQuantity(int direction);

    ProductCatalog&  catalog_;
    OrderManager&    orders_;
    CheckoutService& checkout_;
    QString          lastBarcode_;

    QWidget*        cameraHolder_ = nullptr;
    QLineEdit*      barcodeEdit_ = nullptr;
    QDoubleSpinBox* qtyBox_ = nullptr;
    QLineEdit*      nameEdit_ = nullptr;
    QLineEdit*      contactEdit_ = nullptr;
    QTableWidget*   cart_ = nullptr;
    QLabel *subtotalLbl_ = nullptr, *taxLbl_ = nullptr,
           *discountLbl_ = nullptr, *payableLbl_ = nullptr, *banner_ = nullptr;
    QPushButton *descBtn_ = nullptr, *plusBtn_ = nullptr, *minusBtn_ = nullptr,
                *removeBtn_ = nullptr, *holdBtn_ = nullptr, *heldBtn_ = nullptr,
                *payBtn_ = nullptr, *cancelBtn_ = nullptr;
    QObject* camera_ = nullptr;   // CameraPanel (M5) when available
};
