#include <QApplication>
#include <QLabel>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QLabel label("Qt works!");
    label.resize(300, 100);
    label.show();
    return app.exec();
}