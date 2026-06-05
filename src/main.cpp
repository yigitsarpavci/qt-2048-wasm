#include <QApplication>
#include <ctime>
#include <cstdlib>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    std::srand(std::time(nullptr));
    QApplication app(argc, argv);
    app.setApplicationName("2048 - CmpE 230");
    app.setOrganizationName("CMPE230");

    MainWindow window;
    window.show();
    window.setFocus(); // Explicitly set focus on startup

    return app.exec();
}
