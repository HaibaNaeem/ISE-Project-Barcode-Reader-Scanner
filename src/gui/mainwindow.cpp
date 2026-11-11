#include "mainwindow.h"
#include "BarcodeGenerator.h"
#include "CameraScanner.h"

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QPixmap>
#include <QFrame>
#include <QFont>
#include <QSpacerItem>
#include <QScrollArea>
#include <QSizePolicy>

#include <opencv2/imgcodecs.hpp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    setWindowTitle("SuperStore Barcode System");

    setMinimumSize(1000, 650);

    resize(1200, 750);

    QWidget *central = new QWidget(this);

    setCentralWidget(central);

    QHBoxLayout *mainLayout = new QHBoxLayout(central);

    mainLayout->setContentsMargins(0, 0, 0, 0);

    mainLayout->setSpacing(0);

    // Create sidebar

    createSidebar();

    // Create application pages

    createPages();

    // Main layout

    QWidget *sidebar = new QWidget();

    sidebar->setFixedWidth(190);

    QVBoxLayout *sideLayout = new QVBoxLayout(sidebar);

    sideLayout->setContentsMargins(10, 20, 10, 15);

    // Logo

    QLabel *logo = new QLabel("▥");

    logo->setAlignment(Qt::AlignCenter);

    logo->setStyleSheet(
        "color: #A855F7;"
        "font-size: 42px;"
        "font-weight: bold;"
        );

    QLabel *appName = new QLabel(
        "SuperStore\nBarcode System"
        );

    appName->setAlignment(Qt::AlignCenter);

    appName->setStyleSheet(
        "color: white;"
        "font-size: 16px;"
        "font-weight: bold;"
        );

    sideLayout->addWidget(logo);

    sideLayout->addWidget(appName);

    sideLayout->addSpacing(35);

    // Navigation buttons

    sideLayout->addWidget(dashboardBtn);

    sideLayout->addWidget(generateBtn);

    sideLayout->addWidget(scanBtn);

    sideLayout->addStretch();

    // Separator

    QFrame *line = new QFrame();

    line->setFrameShape(QFrame::HLine);

    line->setStyleSheet("color: #39344F;");

    sideLayout->addWidget(line);

    sideLayout->addSpacing(15);

    sideLayout->addWidget(settingsBtn);

    sideLayout->addWidget(aboutBtn);

    // Add sidebar and pages

    mainLayout->addWidget(sidebar);

    mainLayout->addWidget(pages);

    // Application background

    central->setStyleSheet(
        "background-color: #12111F;"
        );

    sidebar->setStyleSheet(
        "background-color: #0D0C18;"
        );

    // Button styling

    QString buttonStyle =
        "QPushButton {"
        "color: white;"
        "background-color: transparent;"
        "border: none;"
        "border-radius: 10px;"
        "padding: 13px;"
        "text-align: left;"
        "font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "background-color: #30204D;"
        "}"
        "QPushButton:checked {"
        "background-color: #6325D9;"
        "color: white;"
        "font-weight: bold;"
        "}";

    dashboardBtn->setStyleSheet(buttonStyle);

    generateBtn->setStyleSheet(buttonStyle);

    scanBtn->setStyleSheet(buttonStyle);

    settingsBtn->setStyleSheet(buttonStyle);

    aboutBtn->setStyleSheet(buttonStyle);

    // Navigation functionality

    connect(dashboardBtn, &QPushButton::clicked,
            this, [this]() { changePage(0); });

    connect(generateBtn, &QPushButton::clicked,
            this, [this]() { changePage(1); });

    connect(scanBtn, &QPushButton::clicked,
            this, [this]() { changePage(2); });

    connect(settingsBtn, &QPushButton::clicked,
            this, [this]() { changePage(3); });

    connect(aboutBtn, &QPushButton::clicked,
            this, [this]() { changePage(4); });

    changePage(0);
}

void MainWindow::createSidebar()
{
    dashboardBtn = new QPushButton("⌂   Dashboard");

    generateBtn = new QPushButton("▥   Generate Barcode");

    scanBtn = new QPushButton("▣   Scan Barcode");

    settingsBtn = new QPushButton("⚙   Settings");

    aboutBtn = new QPushButton("ⓘ   About");

    dashboardBtn->setCheckable(true);

    generateBtn->setCheckable(true);

    scanBtn->setCheckable(true);

    settingsBtn->setCheckable(true);

    aboutBtn->setCheckable(true);
}

void MainWindow::createPages()
{
    pages = new QStackedWidget();

    QString pageStyle =
        "background-color: #171626;"
        "color: white;";

    QString labelStyle =
        "color: white;"
        "font-size: 26px;"
        "font-weight: bold;";

    // Dashboard


    // DASHBOARD PAGE

    QWidget *dashboard = new QWidget();

    QVBoxLayout *dashboardLayout = new QVBoxLayout(dashboard);

    dashboardLayout->setContentsMargins(25, 20, 25, 25);

    dashboardLayout->setSpacing(20);


    // TOP HEADER

    QLabel *dashboardTitle = new QLabel("⌂   Dashboard");

    dashboardTitle->setStyleSheet(
        "color: white;"
        "font-size: 25px;"
        "font-weight: bold;"
        );

    dashboardLayout->addWidget(dashboardTitle);


    // WELCOME SECTION

    QFrame *welcomeFrame = new QFrame();

    welcomeFrame->setMinimumHeight(190);

    welcomeFrame->setStyleSheet(
        "QFrame {"
        "background-color: #211B3B;"
        "border-radius: 18px;"
        "border: 1px solid #39275F;"
        "}"
        );

    QHBoxLayout *welcomeLayout = new QHBoxLayout(welcomeFrame);

    QLabel *scannerIcon = new QLabel("▥");

    scannerIcon->setAlignment(Qt::AlignCenter);

    scannerIcon->setMinimumSize(180, 150);

    scannerIcon->setStyleSheet(
        "color: #A855F7;"
        "font-size: 100px;"
        "font-weight: bold;"
        "border: none;"
        );

    QVBoxLayout *welcomeText = new QVBoxLayout();

    QLabel *welcome = new QLabel("Welcome to");

    welcome->setStyleSheet(
        "color: white;"
        "font-size: 25px;"
        "font-weight: bold;"
        "border: none;"
        );

    QLabel *systemName = new QLabel(
        "SuperStore Barcode System"
        );

    systemName->setStyleSheet(
        "color: #C084FC;"
        "font-size: 25px;"
        "font-weight: bold;"
        "border: none;"
        );

    QLabel *description = new QLabel(
        "Manage your product barcodes in one place."
        );

    description->setStyleSheet(
        "color: #D1D5DB;"
        "font-size: 13px;"
        "border: none;"
        );

    welcomeText->addWidget(welcome);
    welcomeText->addWidget(systemName);
    welcomeText->addWidget(description);

    welcomeLayout->addWidget(scannerIcon, 1);

    welcomeLayout->addLayout(welcomeText, 2);

    welcomeLayout->setAlignment(
        Qt::AlignVCenter
        );

    dashboardLayout->addWidget(welcomeFrame);


    // FEATURE CARDS

    QHBoxLayout *cardsLayout = new QHBoxLayout();

    cardsLayout->setSpacing(15);


    // GENERATE CARD

    QFrame *generateCard = new QFrame();

    generateCard->setStyleSheet(
        "QFrame {"
        "background-color: #25203D;"
        "border: 1px solid #49366D;"
        "border-radius: 15px;"
        "}"
        );

    QVBoxLayout *generateCardLayout =
        new QVBoxLayout(generateCard);

    QLabel *generateIcon = new QLabel("▥");

    generateIcon->setAlignment(Qt::AlignCenter);
    generateIcon->setFixedHeight(90);

    generateIcon->setStyleSheet(
        "color: #C084FC;"
        "font-size: 75px;"
        "font-weight: bold;"
        "border: none;"
        );

    generateIcon->setAlignment(Qt::AlignCenter);

    generateIcon->setFixedHeight(90);

    generateIcon->setStyleSheet(
        "color: #C084FC;"
        "font-size: 75px;"
        "font-weight: bold;"
        "border: none;"
        );

    generateIcon->setStyleSheet(
        "color: #C084FC;"
        "font-size: 55px;"
        "border: none;"
        );

    QLabel *generateHeading = new QLabel("Generate");

    generateHeading->setAlignment(Qt::AlignCenter);

    generateHeading->setStyleSheet(
        "color: white;"
        "font-size: 21px;"
        "font-weight: bold;"
        "border: none;"
        );

    QLabel *generateDescription = new QLabel(
        "Create product barcodes"
        );

    generateDescription->setAlignment(Qt::AlignCenter);

    generateDescription->setStyleSheet(
        "color: #D1D5DB;"
        "font-size: 13px;"
        "border: none;"
        );

    QPushButton *goGenerate = new QPushButton(
        "Go to Generate  →"
        );

    goGenerate->setStyleSheet(
        "QPushButton {"
        "background-color: #9333EA;"
        "color: white;"
        "border-radius: 18px;"
        "padding: 10px;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "background-color: #A855F7;"
        "}"
        );

    generateCardLayout->addWidget(
        generateIcon, 0, Qt::AlignHCenter
        );
    generateCardLayout->addWidget(generateHeading);
    generateCardLayout->addWidget(generateDescription);
    generateCardLayout->addWidget(goGenerate);


    // SCAN CARD

    QFrame *scanCard = new QFrame();

    scanCard->setStyleSheet(
        "QFrame {"
        "background-color: #202D4C;"
        "border: 1px solid #304D78;"
        "border-radius: 15px;"
        "}"
        );

    QVBoxLayout *scanCardLayout =
        new QVBoxLayout(scanCard);

    QLabel *scanIcon = new QLabel("▣");

    scanIcon->setAlignment(Qt::AlignCenter);

    scanIcon->setFixedHeight(90);

    scanIcon->setStyleSheet(
        "color: #38BDF8;"
        "font-size: 75px;"
        "font-weight: bold;"
        "border: none;"
        );



    QLabel *scanHeading = new QLabel("Scan");

    scanHeading->setAlignment(Qt::AlignCenter);

    scanHeading->setStyleSheet(
        "color: white;"
        "font-size: 21px;"
        "font-weight: bold;"
        "border: none;"
        );

    QLabel *scanDescription = new QLabel(
        "Upload or use camera"
        );

    scanDescription->setAlignment(Qt::AlignCenter);

    scanDescription->setStyleSheet(
        "color: #D1D5DB;"
        "font-size: 13px;"
        "border: none;"
        );

    QPushButton *goScan = new QPushButton(
        "Go to Scan  →"
        );

    goScan->setStyleSheet(
        "QPushButton {"
        "background-color: #1689E8;"
        "color: white;"
        "border-radius: 18px;"
        "padding: 10px;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "background-color: #38BDF8;"
        "}"
        );

    scanCardLayout->addWidget(scanIcon);
    scanCardLayout->addWidget(scanHeading);
    scanCardLayout->addWidget(scanDescription);
    scanCardLayout->addWidget(goScan);


    // ADD CARDS

    cardsLayout->addWidget(generateCard);
    cardsLayout->addWidget(scanCard);

    dashboardLayout->addLayout(cardsLayout);

    dashboardLayout->addStretch();


    // CARD BUTTON FUNCTIONALITY

    connect(goGenerate, &QPushButton::clicked,
            this, [this]() { changePage(1); });

    connect(goScan, &QPushButton::clicked,
            this, [this]() { changePage(2); });

    dashboard->setStyleSheet(
        "background-color: #171626;"
        );

    // Generate Barcode

    // GENERATE BARCODE PAGE

    QWidget *generate = new QWidget();

    QVBoxLayout *generateLayout = new QVBoxLayout(generate);

    generateLayout->setContentsMargins(30, 25, 30, 25);

    generateLayout->setSpacing(20);


    // PAGE HEADING

    QLabel *generateLabel = new QLabel("▥   Generate Barcode");

    generateLabel->setStyleSheet(
        "color: white;"
        "font-size: 26px;"
        "font-weight: bold;"
        );

    generateLayout->addWidget(generateLabel);


    // DESCRIPTION

    QLabel *generateInfo = new QLabel(
        "Create unique barcodes for your store products."
        );

    generateInfo->setStyleSheet(
        "color: #A5A3B5;"
        "font-size: 14px;"
        );

    generateLayout->addWidget(generateInfo);


    // INPUT SECTION

    QFrame *inputFrame = new QFrame();

    inputFrame->setStyleSheet(
        "background-color: #25203D;"
        "border: 1px solid #49366D;"
        "border-radius: 15px;"
        );

    QVBoxLayout *inputLayout = new QVBoxLayout(inputFrame);

    inputLayout->setContentsMargins(20, 20, 20, 20);

    QLabel *productLabel = new QLabel("Product Code");

    productLabel->setStyleSheet(
        "color: white;"
        "font-size: 16px;"
        "font-weight: bold;"
        "border: none;"
        );

    QLineEdit *productInput = new QLineEdit();

    productInput->setPlaceholderText("Enter 6-digit product code");

    productInput->setMaxLength(6);

    productInput->setStyleSheet(
        "QLineEdit {"
        "background-color: #171626;"
        "color: white;"
        "border: 1px solid #62429A;"
        "border-radius: 8px;"
        "padding: 12px;"
        "font-size: 15px;"
        "}"
        "QLineEdit:focus {"
        "border: 1px solid #A855F7;"
        "}"
        );

    QPushButton *generateBarcodeBtn =
        new QPushButton("Generate Barcode");

    generateBarcodeBtn->setStyleSheet(
        "QPushButton {"
        "background-color: #9333EA;"
        "color: white;"
        "border: none;"
        "border-radius: 10px;"
        "padding: 13px;"
        "font-size: 14px;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "background-color: #A855F7;"
        "}"
        );

    inputLayout->addWidget(productLabel);

    inputLayout->addWidget(productInput);

    inputLayout->addSpacing(10);

    inputLayout->addWidget(generateBarcodeBtn);

    generateLayout->addWidget(inputFrame);


    // BARCODE PREVIEW SECTION

    QFrame *previewFrame = new QFrame();

    previewFrame->setStyleSheet(
        "background-color: #25203D;"
        "border: 1px solid #49366D;"
        "border-radius: 15px;"
        );

    QVBoxLayout *previewLayout = new QVBoxLayout(previewFrame);

    QLabel *previewHeading = new QLabel("Barcode Preview");

    previewHeading->setStyleSheet(
        "color: white;"
        "font-size: 18px;"
        "font-weight: bold;"
        "border: none;"
        );

    QLabel *barcodePreview = new QLabel(
        "Your generated barcode will appear here"
        );

    barcodePreview->setAlignment(Qt::AlignCenter);

    barcodePreview->setMinimumHeight(130);

    barcodePreview->setStyleSheet(
        "background-color: #171626;"
        "color: #A5A3B5;"
        "border: 1px dashed #62429A;"
        "border-radius: 10px;"
        "font-size: 14px;"
        );

    QPushButton *saveBarcodeBtn = new QPushButton("Save Barcode");

    saveBarcodeBtn->setEnabled(false);

    saveBarcodeBtn->setStyleSheet(
        "QPushButton {"
        "background-color: #1689E8;"
        "color: white;"
        "border: none;"
        "border-radius: 10px;"
        "padding: 12px;"
        "font-weight: bold;"
        "}"
        "QPushButton:disabled {"
        "background-color: #454154;"
        "color: #999999;"
        "}"
        );

    QLabel *statusLabel = new QLabel("Ready to generate barcode.");

    statusLabel->setStyleSheet(
        "color: #A5A3B5;"
        "font-size: 13px;"
        "border: none;"
        );

    previewLayout->addWidget(previewHeading);

    previewLayout->addWidget(barcodePreview);

    previewLayout->addWidget(saveBarcodeBtn);

    previewLayout->addWidget(statusLabel);

    generateLayout->addWidget(previewFrame);

    generateLayout->addStretch();

    generate->setStyleSheet(
        "background-color: #171626;"
        );


    // GENERATE BUTTON VALIDATION

    connect(generateBarcodeBtn, &QPushButton::clicked,
            this, [=]() {

                QString code = productInput->text();

                if (code.length() != 6) {

                    statusLabel->setText(
                        "Please enter a valid 6-digit product code."
                        );

                    statusLabel->setStyleSheet(
                        "color: #F87171;"
                        "font-size: 13px;"
                        );

                    return;
                }

                const QString outputPath = QDir::current().filePath(
                    QString("output/generated_barcodes/%1.bmp").arg(code));
                const OperationResult result = generateBarcode(
                    code.toStdString(), outputPath.toStdString());

                if (!result.success) {
                    barcodePreview->setText(QString::fromStdString(result.message));
                    saveBarcodeBtn->setEnabled(false);
                    statusLabel->setText(QString::fromStdString(result.message));
                    statusLabel->setStyleSheet("color: #F87171; font-size: 13px;");
                    return;
                }

                barcodePreview->setPixmap(QPixmap(outputPath).scaled(
                    500, 140, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                barcodePreview->setProperty("generatedPath", outputPath);
                saveBarcodeBtn->setEnabled(true);
                statusLabel->setText("Barcode generated successfully.");
                statusLabel->setStyleSheet("color: #A78BFA; font-size: 13px;");

            });

    connect(saveBarcodeBtn, &QPushButton::clicked,
            this, [=]() {
                const QString sourcePath = barcodePreview->property("generatedPath").toString();
                if (sourcePath.isEmpty())
                    return;

                const QString destinationPath = QFileDialog::getSaveFileName(
                    this, "Save Barcode", QFileInfo(sourcePath).fileName(),
                    "Bitmap image (*.bmp)");
                if (destinationPath.isEmpty())
                    return;

                if (!QFile::copy(sourcePath, destinationPath)) {
                    statusLabel->setText("Could not save the barcode image.");
                    statusLabel->setStyleSheet("color: #F87171; font-size: 13px;");
                    return;
                }

                statusLabel->setText("Barcode saved successfully.");
                statusLabel->setStyleSheet("color: #A78BFA; font-size: 13px;");
            });

    // Scan Barcode

    // SCAN BARCODE PAGE

    QWidget *scan = new QWidget();

    QVBoxLayout *scanLayout = new QVBoxLayout(scan);

    scanLayout->setContentsMargins(30, 25, 30, 25);

    scanLayout->setSpacing(18);


    // PAGE HEADING

    QLabel *scanLabel = new QLabel("▣   Scan Barcode");

    scanLabel->setStyleSheet(
        "color: white;"
        "font-size: 26px;"
        "font-weight: bold;"
        );

    scanLayout->addWidget(scanLabel);


    // DESCRIPTION

    QLabel *scanInfo = new QLabel(
        "Upload a barcode image to identify your product."
        );

    scanInfo->setStyleSheet(
        "color: #A5A3B5;"
        "font-size: 14px;"
        );

    scanLayout->addWidget(scanInfo);


    // UPLOAD SECTION

    QFrame *uploadFrame = new QFrame();

    uploadFrame->setStyleSheet(
        "background-color: #25203D;"
        "border: 1px solid #49366D;"
        "border-radius: 15px;"
        );

    QVBoxLayout *uploadLayout = new QVBoxLayout(uploadFrame);

    uploadLayout->setContentsMargins(20, 20, 20, 20);

    uploadLayout->setSpacing(12);


    // UPLOAD HEADING

    QLabel *uploadHeading = new QLabel("Upload Barcode Image");

    uploadHeading->setStyleSheet(
        "color: white;"
        "font-size: 18px;"
        "font-weight: bold;"
        "border: none;"
        );

    uploadLayout->addWidget(uploadHeading);


    // IMAGE PREVIEW

    QLabel *imagePreview = new QLabel(
        "No image selected\n\nBrowse and upload a barcode image"
        );

    imagePreview->setAlignment(Qt::AlignCenter);

    imagePreview->setFixedHeight(160);

    imagePreview->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Fixed
        );

    imagePreview->setStyleSheet(
        "background-color: #171626;"
        "color: #A5A3B5;"
        "border: 2px dashed #62429A;"
        "border-radius: 10px;"
        "font-size: 14px;"
        );

    uploadLayout->addWidget(imagePreview);


    // BROWSE BUTTON

    QPushButton *browseBtn = new QPushButton("Browse Image");

    browseBtn->setStyleSheet(
        "QPushButton {"
        "background-color: #9333EA;"
        "color: white;"
        "border: none;"
        "border-radius: 10px;"
        "padding: 12px;"
        "font-size: 14px;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "background-color: #A855F7;"
        "}"
        );

    uploadLayout->addWidget(browseBtn);


    // SCAN BUTTON

    QPushButton *scanBarcodeBtn = new QPushButton("Scan Barcode");

    scanBarcodeBtn->setEnabled(false);

    scanBarcodeBtn->setStyleSheet(
        "QPushButton {"
        "background-color: #1689E8;"
        "color: white;"
        "border: none;"
        "border-radius: 10px;"
        "padding: 12px;"
        "font-size: 14px;"
        "font-weight: bold;"
        "}"
        "QPushButton:disabled {"
        "background-color: #454154;"
        "color: #999999;"
        "}"
        );

    uploadLayout->addWidget(scanBarcodeBtn);

    scanLayout->addWidget(uploadFrame);
    // CAMERA SCANNER SECTION

    QFrame *cameraFrame = new QFrame();

    cameraFrame->setStyleSheet(
        "background-color: #25203D;"
        "border: 1px solid #49366D;"
        "border-radius: 15px;"
        );

    QVBoxLayout *cameraLayout = new QVBoxLayout(cameraFrame);

    cameraLayout->setContentsMargins(20, 20, 20, 20);
    cameraLayout->setSpacing(10);


    // CAMERA HEADING

    QLabel *cameraHeading = new QLabel("Live Camera Scanner");

    cameraHeading->setStyleSheet(
        "color: white;"
        "font-size: 18px;"
        "font-weight: bold;"
        "border: none;"
        );

    cameraLayout->addWidget(cameraHeading);


    // CAMERA PREVIEW AREA

    QLabel *cameraPreview = new QLabel(
        "Camera is currently OFF\n\n"
        "Your camera preview will appear here."
        );

    cameraPreview->setAlignment(Qt::AlignCenter);

    cameraPreview->setFixedHeight(170);

    cameraPreview->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Fixed
        );

    cameraPreview->setStyleSheet(
        "background-color: #171626;"
        "color: #A5A3B5;"
        "border: 2px dashed #62429A;"
        "border-radius: 10px;"
        "font-size: 14px;"
        );

    cameraLayout->addWidget(cameraPreview);


    // CAMERA STATUS

    QLabel *cameraStatus = new QLabel("● Camera Status: Not Connected");

    cameraStatus->setStyleSheet(
        "color: #F87171;"
        "font-size: 13px;"
        "border: none;"
        );

    cameraLayout->addWidget(cameraStatus);


    // CAMERA BUTTONS

    QHBoxLayout *cameraButtonLayout = new QHBoxLayout();

    QPushButton *startCameraBtn = new QPushButton("Start Camera");

    QPushButton *stopCameraBtn = new QPushButton("Stop Camera");

    stopCameraBtn->setEnabled(false);


    QString cameraButtonStyle =
        "QPushButton {"
        "background-color: #9333EA;"
        "color: white;"
        "border: none;"
        "border-radius: 10px;"
        "padding: 12px;"
        "font-size: 14px;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "background-color: #A855F7;"
        "}"
        "QPushButton:disabled {"
        "background-color: #454154;"
        "color: #999999;"
        "}";

    startCameraBtn->setStyleSheet(cameraButtonStyle);

    stopCameraBtn->setStyleSheet(cameraButtonStyle);

    cameraButtonLayout->addWidget(startCameraBtn);

    cameraButtonLayout->addWidget(stopCameraBtn);

    cameraLayout->addLayout(cameraButtonLayout);

    scanLayout->addWidget(cameraFrame);


    // START CAMERA BUTTON

    connect(startCameraBtn, &QPushButton::clicked,
            this, [=]() {

                cameraPreview->setText(
                    "Camera interface is ready.\n\n"
                    "Waiting for webcam integration."
                    );

                cameraStatus->setText(
                    "● Camera Status: Ready for connection"
                    );

                cameraStatus->setStyleSheet(
                    "color: #FBBF24;"
                    "font-size: 13px;"
                    "border: none;"
                    );

                startCameraBtn->setEnabled(false);

                stopCameraBtn->setEnabled(true);

            });


    // STOP CAMERA BUTTON

    connect(stopCameraBtn, &QPushButton::clicked,
            this, [=]() {

                cameraPreview->setText(
                    "Camera is currently OFF\n\n"
                    "Your camera preview will appear here."
                    );

                cameraStatus->setText(
                    "● Camera Status: Not Connected"
                    );

                cameraStatus->setStyleSheet(
                    "color: #F87171;"
                    "font-size: 13px;"
                    "border: none;"
                    );

                startCameraBtn->setEnabled(true);

                stopCameraBtn->setEnabled(false);

            });


    // RESULT SECTION

    QFrame *resultFrame = new QFrame();

    resultFrame->setStyleSheet(
        "background-color: #25203D;"
        "border: 1px solid #49366D;"
        "border-radius: 15px;"
        );

    QVBoxLayout *resultLayout = new QVBoxLayout(resultFrame);

    QLabel *resultHeading = new QLabel("Scan Result");

    resultHeading->setStyleSheet(
        "color: white;"
        "font-size: 18px;"
        "font-weight: bold;"
        "border: none;"
        );

    QLabel *resultLabel = new QLabel(
        "Scanned product code will appear here."
        );

    resultLabel->setStyleSheet(
        "background-color: #171626;"
        "color: #A5A3B5;"
        "border-radius: 8px;"
        "padding: 15px;"
        "font-size: 14px;"
        );

    resultLayout->addWidget(resultHeading);

    resultLayout->addWidget(resultLabel);

    scanLayout->addWidget(resultFrame);

    scanLayout->addStretch();

    scan->setStyleSheet(
        "background-color: #171626;"
        );


    // IMAGE UPLOAD FUNCTIONALITY

    connect(browseBtn, &QPushButton::clicked,
            this, [=]() {

                QString fileName = QFileDialog::getOpenFileName(
                    this,
                    "Select Barcode Image",
                    "",
                    "Images (*.png *.jpg *.jpeg *.bmp)"
                    );

                if (fileName.isEmpty()) {
                    return;
                }

                QPixmap image(fileName);

                if (image.isNull()) {
                    resultLabel->setText("Unable to load selected image.");
                    scanBarcodeBtn->setEnabled(false);
                    return;
                }

                imagePreview->setPixmap(
                    image.scaled(
                        500, 200,
                        Qt::KeepAspectRatio,
                        Qt::SmoothTransformation
                        )
                    );

                scanBarcodeBtn->setEnabled(true);
                imagePreview->setProperty("sourcePath", fileName);

                resultLabel->setText(
                    "Image uploaded successfully. Ready to scan."
                    );

            });


    // SCAN BUTTON

    connect(scanBarcodeBtn, &QPushButton::clicked,
            this, [=]() {

                const QString imagePath = imagePreview->property("sourcePath").toString();
                const cv::Mat image = cv::imread(imagePath.toLocal8Bit().constData(),
                                                 cv::IMREAD_UNCHANGED);
                const OperationResult result = CameraScanner::decodeFrame(image);
                resultLabel->setText(result.success
                    ? QString("Scanned product code: %1").arg(QString::fromStdString(result.value))
                    : QString::fromStdString(result.message));

            });

    // Settings

    QWidget *settings = new QWidget();

    QLabel *settingsLabel = new QLabel("⚙   Settings");

    settingsLabel->setStyleSheet(labelStyle);

    QVBoxLayout *settingsLayout = new QVBoxLayout(settings);

    settingsLayout->addWidget(settingsLabel);

    settingsLayout->addStretch();

    settings->setStyleSheet(pageStyle);

    // About

    QWidget *about = new QWidget();

    QLabel *aboutLabel = new QLabel("ⓘ   About");

    aboutLabel->setStyleSheet(labelStyle);

    QVBoxLayout *aboutLayout = new QVBoxLayout(about);

    aboutLayout->addWidget(aboutLabel);

    aboutLayout->addStretch();

    about->setStyleSheet(pageStyle);

    // Add all pages

    // Add all pages

    pages->addWidget(dashboard);

    pages->addWidget(generate);

    // SCAN PAGE SCROLL AREA

    QScrollArea *scanScroll = new QScrollArea();

    scanScroll->setWidgetResizable(true);

    scanScroll->setFrameShape(QFrame::NoFrame);

    scanScroll->setHorizontalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff
        );

    scanScroll->setVerticalScrollBarPolicy(
        Qt::ScrollBarAsNeeded
        );

    scan->setMinimumWidth(0);

    scanLayout->setSizeConstraint(
        QLayout::SetMinimumSize
        );

    scanScroll->setWidget(scan);

    pages->addWidget(scanScroll);

    pages->addWidget(settings);

    pages->addWidget(about);
}

void MainWindow::changePage(int index)
{
    pages->setCurrentIndex(index);

    dashboardBtn->setChecked(index == 0);

    generateBtn->setChecked(index == 1);

    scanBtn->setChecked(index == 2);

    settingsBtn->setChecked(index == 3);

    aboutBtn->setChecked(index == 4);
}
