#include <QApplication>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QSettings>

#include "mainwindow.h"
#include "floatingball.h"
#include "welcomedialog.h"
#include "configmanager.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    QCoreApplication::setOrganizationName("Classassistant");
    QCoreApplication::setApplicationName("Classassistant");

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, QObject::tr("错误"), QObject::tr("系统不支持托盘功能，程序无法运行！"));
        return 1;
    }

    qRegisterMetaType<ButtonConfig>("ButtonConfig");
    ConfigManager::instance();

    QSettings settings;
    bool firstRun = settings.value("General/FirstRun", true).toBool();
    if (firstRun) {
        WelcomeDialog welcome;
        if (welcome.exec() == QDialog::Accepted) settings.setValue("General/FirstRun", false);
        else return 0;
    }

    auto *ball = new FloatingBall();
    auto *mainWindow = new MainWindow();
    ball->show();

    QObject::connect(ball, &FloatingBall::clicked, mainWindow, &MainWindow::showSidebar);
    QObject::connect(mainWindow, &MainWindow::sidebarHidden, ball, &FloatingBall::show);

    return app.exec();
}
