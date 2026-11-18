#include "MainWindow.h"
#include "OrderPage.h"

#include <QCoreApplication>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      paths_(QCoreApplication::applicationDirPath().toStdString()),
      catalog_(),
      store_(),
      bills_(paths_.billFile()),
      orders_(catalog_, &store_),
      checkout_(catalog_, store_, bills_, paths_.productFile()) {
    setWindowTitle("PoS V1");

    paths_.ensureDataFolder();
    catalog_.load(paths_.productFile());
    store_ = OrderStore(paths_.orderFile(), paths_.orderDetailFile());
    store_.load();
    bills_.load();

    page_ = new OrderPage(catalog_, orders_, checkout_, this);
    setCentralWidget(page_);
}