#pragma once
#include <QMainWindow>
#include "core/DataPaths.h"
#include "core/ProductCatalog.h"
#include "core/OrderStore.h"
#include "core/OrderManager.h"
#include "core/CheckoutService.h"

class OrderPage;

// Creates the core objects once and gives them to the OrderPage.
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    // Declaration order = construction order. Do not reorder.
    DataPaths       paths_;
    ProductCatalog  catalog_;
    OrderStore      store_;
    OrderManager    orders_;
    CheckoutService checkout_;
    OrderPage*      page_ = nullptr;
};
