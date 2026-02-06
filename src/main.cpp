#include "appcontroller.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("Classassistant");
    QApplication::setOrganizationName("Classassistant");

    AppController controller;
    controller.start();

    return app.exec();
}
