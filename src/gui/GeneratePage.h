#pragma once
#include <QWidget>
#include <QPixmap>

class QLineEdit;
class QPushButton;
class QLabel;

// GeneratePage (Member 1 - Desktop GUI)
// Provides UI to enter 6-digit codes, validate input, call Member 2's
// barcode generator, display the preview, and save the barcode image.
class GeneratePage : public QWidget {
    Q_OBJECT

public:
    explicit GeneratePage(QWidget* parent = nullptr);

private slots:
    void onGenerateClicked();
    void onSaveClicked();
    void onClearClicked();
    void onInputTextChanged(const QString& text);

private:
    void setupUi();
    void showStatus(const QString& message, const QString& type);
    void clearStatus();

    QLineEdit*   m_codeInput          = nullptr;
    QPushButton* m_generateBtn        = nullptr;
    QPushButton* m_saveBtn            = nullptr;
    QPushButton* m_clearBtn           = nullptr;
    QLabel*      m_previewLabel       = nullptr;
    QLabel*      m_statusLabel        = nullptr;
    QLabel*      m_detailsLabel       = nullptr;

    QPixmap      m_currentPixmap;
    QString      m_currentCode;
    QString      m_currentFilePath;
};
