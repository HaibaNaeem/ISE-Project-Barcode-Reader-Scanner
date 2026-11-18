#pragma once
// OWNER: Member 1. Main Application Window.
#include <QMainWindow>
#include <memory>

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
    ~MainWindow() override;

private:
    DataPaths paths_;
    ProductCatalog catalog_;
    OrderStore store_;
    BillStore bills_;
    std::unique_ptr<OrderManager> orders_;
    std::unique_ptr<CheckoutService> checkout_;
    OrderPage* page_ = nullptr;
};
