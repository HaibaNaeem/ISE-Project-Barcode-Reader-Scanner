#include "MainWindow.h"
#include "GeneratePage.h"
#include "ScanPage.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setupUi();
}

void MainWindow::setupUi()
{
    setWindowTitle("Barcode Reader & Scanner");
    resize(1060, 720);
    setMinimumSize(880, 600);

    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto* rootLayout = new QHBoxLayout(centralWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // Sidebar Navigation Panel
    auto* navBar = new QFrame(centralWidget);
    navBar->setObjectName("navBar");
    auto* navLayout = new QVBoxLayout(navBar);
    navLayout->setContentsMargins(14, 24, 14, 24);
    navLayout->setSpacing(8);

    // App Branding Header
    auto* brandLayout = new QVBoxLayout();
    brandLayout->setContentsMargins(10, 0, 10, 16);
    brandLayout->setSpacing(2);

    auto* appTitle = new QLabel("Barcode Studio", navBar);
    appTitle->setObjectName("appTitle");
    appTitle->setWordWrap(true);

    auto* appSubtitle = new QLabel("ISE Barcode Reader & Scanner", navBar);
    appSubtitle->setObjectName("appSubtitle");

    brandLayout->addWidget(appTitle);
    brandLayout->addWidget(appSubtitle);
    navLayout->addLayout(brandLayout);

    // Navigation Buttons
    m_navGenerateBtn = new QPushButton("  1.  Generate Barcode", navBar);
    m_navGenerateBtn->setProperty("class", "navButton");
    m_navGenerateBtn->setCheckable(true);
    m_navGenerateBtn->setChecked(true);
    m_navGenerateBtn->setCursor(Qt::PointingHandCursor);

    m_navScanBtn = new QPushButton("  2.  Scan Barcode", navBar);
    m_navScanBtn->setProperty("class", "navButton");
    m_navScanBtn->setCheckable(true);
    m_navScanBtn->setChecked(false);
    m_navScanBtn->setCursor(Qt::PointingHandCursor);

    navLayout->addWidget(m_navGenerateBtn);
    navLayout->addWidget(m_navScanBtn);

    navLayout->addStretch();

    // Footer Info
    auto* footerLabel = new QLabel("v1.0  |  Qt 6 & C++17\nMember 1: Desktop GUI", navBar);
    footerLabel->setStyleSheet("color: #475569; font-size: 11px; padding: 10px;");
    footerLabel->setAlignment(Qt::AlignLeft);
    navLayout->addWidget(footerLabel);

    rootLayout->addWidget(navBar);

    // Content Pages (Stacked Widget)
    m_stackedWidget = new QStackedWidget(centralWidget);

    m_generatePage = new GeneratePage(m_stackedWidget);
    m_scanPage = new ScanPage(m_stackedWidget);

    m_stackedWidget->addWidget(m_generatePage); // Index 0
    m_stackedWidget->addWidget(m_scanPage);     // Index 1

    rootLayout->addWidget(m_stackedWidget, 1);

    // Event Connections
    connect(m_navGenerateBtn, &QPushButton::clicked, this, &MainWindow::showGeneratePage);
    connect(m_navScanBtn, &QPushButton::clicked, this, &MainWindow::showScanPage);
}

void MainWindow::showGeneratePage()
{
    // Stop webcam if it is running when navigating away from scan page
    if (m_scanPage) {
        m_scanPage->stopCameraIfRunning();
    }

    m_stackedWidget->setCurrentIndex(0);
    m_navGenerateBtn->setChecked(true);
    m_navScanBtn->setChecked(false);
}

void MainWindow::showScanPage()
{
    m_stackedWidget->setCurrentIndex(1);
    m_navGenerateBtn->setChecked(false);
    m_navScanBtn->setChecked(true);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    // Ensure hardware camera is safely released before window closes
    if (m_scanPage) {
        m_scanPage->stopCameraIfRunning();
    }
    event->accept();
}
