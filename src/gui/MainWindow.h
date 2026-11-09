#pragma once
#include <QMainWindow>

class QStackedWidget;
class QPushButton;
class GeneratePage;
class ScanPage;

// MainWindow (Member 1 - Desktop GUI)
// Hosts the main desktop interface, sidebar navigation, and switches
// between Generate and Scan pages using a QStackedWidget.
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void showGeneratePage();
    void showScanPage();

private:
    void setupUi();

    QStackedWidget* m_stackedWidget   = nullptr;
    QPushButton*    m_navGenerateBtn  = nullptr;
    QPushButton*    m_navScanBtn      = nullptr;

    GeneratePage*   m_generatePage    = nullptr;
    ScanPage*       m_scanPage        = nullptr;
};
