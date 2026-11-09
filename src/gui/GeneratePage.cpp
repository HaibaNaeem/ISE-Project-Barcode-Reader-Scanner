#include "GeneratePage.h"
#include "Theme.h"
#include "BarcodeGenerator.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QDir>
#include <QDateTime>
#include <QFileInfo>

GeneratePage::GeneratePage(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void GeneratePage::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(20);

    // Header section
    auto* headerLayout = new QVBoxLayout();
    headerLayout->setSpacing(4);

    auto* heading = new QLabel("Generate Barcode", this);
    heading->setProperty("class", "pageHeading");

    auto* subtitle = new QLabel("Create a scannable barcode image from a six-digit code.", this);
    subtitle->setProperty("class", "pageSubtitle");

    headerLayout->addWidget(heading);
    headerLayout->addWidget(subtitle);
    mainLayout->addLayout(headerLayout);

    // Input card frame
    auto* inputCard = new QFrame(this);
    inputCard->setProperty("class", "card");
    auto* inputCardLayout = new QVBoxLayout(inputCard);
    inputCardLayout->setContentsMargins(20, 18, 20, 18);
    inputCardLayout->setSpacing(14);

    auto* inputLabel = new QLabel("Enter Six-Digit Code (0-9):", inputCard);
    inputLabel->setProperty("class", "sectionTitle");
    inputCardLayout->addWidget(inputLabel);

    auto* controlsRow = new QHBoxLayout();
    controlsRow->setSpacing(12);

    m_codeInput = new QLineEdit(inputCard);
    m_codeInput->setPlaceholderText("e.g. 123456");
    m_codeInput->setMaxLength(6);
    m_codeInput->setClearButtonEnabled(true);
    m_codeInput->setMinimumHeight(42);

    // Input validator: allow only numeric digits (0-9) up to 6 characters
    QRegularExpression rx("^[0-9]{0,6}$");
    auto* validator = new QRegularExpressionValidator(rx, this);
    m_codeInput->setValidator(validator);

    m_generateBtn = new QPushButton("Generate Barcode", inputCard);
    m_generateBtn->setProperty("class", "primaryBtn");
    m_generateBtn->setMinimumHeight(42);
    m_generateBtn->setCursor(Qt::PointingHandCursor);

    m_clearBtn = new QPushButton("Clear", inputCard);
    m_clearBtn->setProperty("class", "secondaryBtn");
    m_clearBtn->setMinimumHeight(42);
    m_clearBtn->setCursor(Qt::PointingHandCursor);

    controlsRow->addWidget(m_codeInput, 2);
    controlsRow->addWidget(m_generateBtn, 1);
    controlsRow->addWidget(m_clearBtn, 0);
    inputCardLayout->addLayout(controlsRow);

    mainLayout->addWidget(inputCard);

    // Status banner (hidden by default)
    m_statusLabel = new QLabel(this);
    m_statusLabel->setObjectName("statusBanner");
    m_statusLabel->setTextFormat(Qt::RichText);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setVisible(false);
    mainLayout->addWidget(m_statusLabel);

    // Preview Card Frame
    auto* previewCard = new QFrame(this);
    previewCard->setProperty("class", "card");
    auto* previewLayout = new QVBoxLayout(previewCard);
    previewLayout->setContentsMargins(20, 18, 20, 18);
    previewLayout->setSpacing(14);

    auto* previewTitleRow = new QHBoxLayout();
    auto* previewHeading = new QLabel("Barcode Preview", previewCard);
    previewHeading->setProperty("class", "sectionTitle");

    m_saveBtn = new QPushButton("Save Barcode Image", previewCard);
    m_saveBtn->setProperty("class", "secondaryBtn");
    m_saveBtn->setMinimumHeight(36);
    m_saveBtn->setEnabled(false);
    m_saveBtn->setCursor(Qt::PointingHandCursor);

    previewTitleRow->addWidget(previewHeading);
    previewTitleRow->addStretch();
    previewTitleRow->addWidget(m_saveBtn);
    previewLayout->addLayout(previewTitleRow);

    // Barcode image preview display area
    m_previewLabel = new QLabel(previewCard);
    m_previewLabel->setObjectName("previewBox");
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setMinimumHeight(220);
    m_previewLabel->setText("No barcode generated yet.\nEnter a 6-digit code above and click 'Generate Barcode'.");
    previewLayout->addWidget(m_previewLabel);

    m_detailsLabel = new QLabel(previewCard);
    m_detailsLabel->setStyleSheet("color: #64748B; font-size: 12px;");
    m_detailsLabel->setAlignment(Qt::AlignCenter);
    m_detailsLabel->setVisible(false);
    previewLayout->addWidget(m_detailsLabel);

    mainLayout->addWidget(previewCard, 1);

    // Connections
    connect(m_generateBtn, &QPushButton::clicked, this, &GeneratePage::onGenerateClicked);
    connect(m_saveBtn, &QPushButton::clicked, this, &GeneratePage::onSaveClicked);
    connect(m_clearBtn, &QPushButton::clicked, this, &GeneratePage::onClearClicked);
    connect(m_codeInput, &QLineEdit::textChanged, this, &GeneratePage::onInputTextChanged);
    connect(m_codeInput, &QLineEdit::returnPressed, this, &GeneratePage::onGenerateClicked);
}

void GeneratePage::onInputTextChanged(const QString& text)
{
    Q_UNUSED(text);
    // If the user modified the text, clear any old status error to prevent confusion
    if (m_statusLabel->isVisible()) {
        m_statusLabel->setVisible(false);
    }
}

void GeneratePage::onGenerateClicked()
{
    const QString code = m_codeInput->text().trimmed();

    // Input Validation
    if (code.isEmpty()) {
        showStatus("Please enter a six-digit code.", "error");
        m_codeInput->setFocus();
        return;
    }

    if (code.length() != 6) {
        showStatus(QString("Code must be exactly 6 digits. You entered %1 digit(s).").arg(code.length()), "error");
        m_codeInput->setFocus();
        return;
    }

    // Ensure output directory exists
    QString outDir = "output/generated_barcodes";
    QDir().mkpath(outDir);

    QString outBmpPath = QString("%1/%2.bmp").arg(outDir, code);

    // Call Member 2's Barcode Generation Service
    OperationResult result = generateBarcode(code.toStdString(), outBmpPath.toStdString());

    if (!result.success) {
        m_currentPixmap = QPixmap();
        m_previewLabel->setText("Barcode generation failed.");
        m_detailsLabel->setVisible(false);
        m_saveBtn->setEnabled(false);
        showStatus(QString::fromStdString(result.message), "error");
        return;
    }

    // Load generated image into preview
    QPixmap pixmap(outBmpPath);
    if (pixmap.isNull()) {
        showStatus("Barcode was generated but the image file could not be loaded into preview.", "error");
        m_saveBtn->setEnabled(false);
        return;
    }

    m_currentPixmap = pixmap;
    m_currentCode = code;
    m_currentFilePath = outBmpPath;

    // Display scaled pixmap keeping aspect ratio
    QPixmap scaled = m_currentPixmap.scaled(
        QSize(500, 160),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );
    m_previewLabel->setPixmap(scaled);

    m_detailsLabel->setText(
        QString("Code: %1  |  Dimensions: %2 x %3 px  |  File: %4")
        .arg(m_currentCode)
        .arg(m_currentPixmap.width())
        .arg(m_currentPixmap.height())
        .arg(outBmpPath)
    );
    m_detailsLabel->setVisible(true);

    m_saveBtn->setEnabled(true);
    showStatus(QString::fromStdString(result.message) + QString(" (Code: %1)").arg(code), "success");
}

void GeneratePage::onSaveClicked()
{
    if (m_currentPixmap.isNull()) {
        showStatus("No generated barcode to save.", "error");
        return;
    }

    QString defaultFileName = QString("barcode_%1.png").arg(m_currentCode);
    QString selectedFilter;
    QString savePath = QFileDialog::getSaveFileName(
        this,
        "Save Barcode Image",
        defaultFileName,
        "PNG Image (*.png);;BMP Image (*.bmp);;JPEG Image (*.jpg *.jpeg)",
        &selectedFilter
    );

    if (savePath.isEmpty()) {
        return; // User canceled
    }

    // Determine format from extension or selected filter
    QString format = "PNG";
    if (savePath.endsWith(".bmp", Qt::CaseInsensitive)) {
        format = "BMP";
    } else if (savePath.endsWith(".jpg", Qt::CaseInsensitive) || savePath.endsWith(".jpeg", Qt::CaseInsensitive)) {
        format = "JPG";
    }

    bool saved = m_currentPixmap.save(savePath, format.toLatin1().constData());
    if (saved) {
        showStatus(QString("Barcode successfully saved to: %1").arg(QFileInfo(savePath).fileName()), "success");
    } else {
        showStatus(QString("Failed to save barcode image to: %1").arg(savePath), "error");
    }
}

void GeneratePage::onClearClicked()
{
    m_codeInput->clear();
    m_currentPixmap = QPixmap();
    m_currentCode.clear();
    m_currentFilePath.clear();

    m_previewLabel->clear();
    m_previewLabel->setText("No barcode generated yet.\nEnter a 6-digit code above and click 'Generate Barcode'.");
    m_detailsLabel->clear();
    m_detailsLabel->setVisible(false);

    m_saveBtn->setEnabled(false);
    clearStatus();
    m_codeInput->setFocus();
}

void GeneratePage::showStatus(const QString& message, const QString& type)
{
    m_statusLabel->setText(Theme::formatStatusHtml(message, type));
    m_statusLabel->setVisible(true);
}

void GeneratePage::clearStatus()
{
    m_statusLabel->clear();
    m_statusLabel->setVisible(false);
}
