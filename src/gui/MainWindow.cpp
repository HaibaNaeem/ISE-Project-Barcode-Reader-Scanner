#include "MainWindow.h"
#include "OrderPage.h"
#include "DataPaths.h"
#include "ProductCatalog.h"
#include "OrderStore.h"
#include "BillStore.h"
#include "OrderManager.h"
#include "CheckoutService.h"
#include <QCoreApplication>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      paths_(QCoreApplication::applicationDirPath().toStdString()),
      catalog_(),
      store_(),
      bills_(paths_.billFile()),                       // BillStore(path) takes the path
      orders_(catalog_, &store_),
      checkout_(catalog_, store_, bills_, paths_.productFile()) {
    setWindowTitle("PoS V1");

    paths_.ensureDataFolder();                         // copies sample data on first run
    catalog_.load(paths_.productFile());
    store_ = OrderStore(paths_.orderFile(), paths_.orderDetailFile());
    store_.load();
    bills_.load();

    page_ = new OrderPage(catalog_, orders_, checkout_, this);
    setCentralWidget(page_);
}