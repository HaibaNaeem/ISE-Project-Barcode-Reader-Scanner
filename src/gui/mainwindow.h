#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>

class MainWindow : public QMainWindow
{
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:

    QStackedWidget *pages;

    QPushButton *dashboardBtn;
    QPushButton *generateBtn;
    QPushButton *scanBtn;
    QPushButton *settingsBtn;
    QPushButton *aboutBtn;

    void setupUI();
    void createSidebar();
    void createPages();
    void changePage(int index);
};

#endif
