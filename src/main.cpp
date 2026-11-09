#include <QApplication>
#include "MainWindow.h"
#include "Theme.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Barcode Reader & Scanner");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("ISE Team");

    // Apply unified modern application theme
    app.setStyleSheet(Theme::getApplicationStyle());

    MainWindow window;
    window.show();

    return app.exec();
}
