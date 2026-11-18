#pragma once
#include <QMainWindow>
#include <QWidget>

#include "DataPaths.h"
#include "ProductCatalog.h"
#include "OrderStore.h"
#include "BillStore.h"
#include "OrderManager.h"
#include "CheckoutService.h"

class OrderPage;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    DataPaths       paths_;
    ProductCatalog  catalog_;
    OrderStore      store_;
    BillStore       bills_;
    OrderManager    orders_;
    CheckoutService checkout_;
    OrderPage*      page_ = nullptr;
};