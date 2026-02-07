#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QIcon>
#include <QMenu>
#include <QStyle>
#include <QSystemTrayIcon>

#include "ui/FloatingBall.h"
#include "ui/Sidebar.h"

int main(int argc, char* argv[]) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    auto* sidebar = new Sidebar();
    auto* ball = new FloatingBall();

    auto updatePos = [&]() {
        const QRect screen = app.primaryScreen()->availableGeometry();
        sidebar->resize(70, screen.height());
        sidebar->move(screen.width() - 70, 0);
    };
    updatePos();

    auto showSidebar = [&]() {
        updatePos();
        ball->hide();
        sidebar->show();
        sidebar->raise();
    };

    auto showBall = [&]() {
        sidebar->hide();
        ball->show();
    };

    QObject::connect(ball, &FloatingBall::clicked, showSidebar);
    QObject::connect(sidebar, &Sidebar::requestHide, showBall);

    auto* tray = new QSystemTrayIcon(&app);
    QIcon trayIcon(":/assets/icon.png");
    if (trayIcon.isNull()) {
        trayIcon = app.style()->standardIcon(QStyle::SP_ComputerIcon);
    }
    tray->setIcon(trayIcon);
    tray->setToolTip("班级小助手");

    auto* menu = new QMenu();
    auto* actionShow = menu->addAction("显示侧边栏");
    auto* actionQuit = menu->addAction("退出");
    QObject::connect(actionShow, &QAction::triggered, showSidebar);
    QObject::connect(actionQuit, &QAction::triggered, [&]() { app.quit(); });
    tray->setContextMenu(menu);
    tray->show();

    QObject::connect(tray, &QSystemTrayIcon::activated, [&](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            showSidebar();
        }
    });

    showSidebar();

    return app.exec();
}
