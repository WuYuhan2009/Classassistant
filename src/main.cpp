#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QIcon>
#include <QMenu>
#include <QScreen>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QTimer>

#include "Utils.h"
#include "ui/FloatingBall.h"
#include "ui/Sidebar.h"
#include "ui/Tools.h"

namespace {
constexpr int kSidebarWidth = 84;

QIcon loadNamedIcon(const QString& fileName) {
    const QString resolvedPath = Config::instance().resolveIconPath(fileName);
    return QIcon(resolvedPath);
}
}

int main(int argc, char* argv[]) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    if (!Config::instance().firstRunCompleted) {
        FirstRunWizard wizard;
        wizard.exec();
    }
    app.setStyleSheet("");

    auto* sidebar = new Sidebar();
    auto* ball = new FloatingBall();

    auto updatePos = [&]() {
        const QRect screen = app.primaryScreen()->availableGeometry();
        sidebar->resize(kSidebarWidth, qMin(screen.height() - 20, 760));
        sidebar->move(screen.right() - kSidebarWidth + 1, screen.center().y() - sidebar->height() / 2);
        ball->setWindowOpacity(Config::instance().floatingOpacity / 100.0);
    };

    auto showSidebar = [&]() {
        updatePos();
        ball->hide();
        sidebar->show();
        sidebar->raise();
        sidebar->activateWindow();
    };

    auto showBall = [&]() {
        sidebar->hide();
        ball->moveToBottomRight();
        ball->show();
        ball->raise();
    };

    auto reloadAll = [&]() {
        Config::instance().load();
        app.setStyleSheet("");
        sidebar->reloadConfig();
        updatePos();
    };

    QObject::connect(ball, &FloatingBall::clicked, showSidebar);
    QObject::connect(sidebar, &Sidebar::requestHide, showBall);

    auto* tray = new QSystemTrayIcon(&app);
    QIcon trayIcon = loadNamedIcon("icon_settings.png");
    if (trayIcon.isNull()) {
        trayIcon = loadNamedIcon("icon_tray.png");
    }
    if (trayIcon.isNull()) {
        trayIcon = QIcon::fromTheme("applications-education");
    }
    if (trayIcon.isNull()) {
        trayIcon = app.style()->standardIcon(QStyle::SP_ComputerIcon);
    }
    tray->setIcon(trayIcon);
    app.setWindowIcon(trayIcon);
    tray->setToolTip("班级小助手");

    auto* menu = new QMenu();
    auto* actionOpenSettings = menu->addAction("打开设置");
    auto* actionShowSidebar = menu->addAction("展开侧边栏");
    auto* actionReloadConfig = menu->addAction("重载配置");
    menu->addSeparator();
    auto* actionQuit = menu->addAction("退出程序");

    QObject::connect(actionOpenSettings, &QAction::triggered, [sidebar]() { sidebar->openSettings(); });
    QObject::connect(actionShowSidebar, &QAction::triggered, showSidebar);
    QObject::connect(actionReloadConfig, &QAction::triggered, reloadAll);
    QObject::connect(actionQuit, &QAction::triggered, [&]() { app.quit(); });

    tray->setContextMenu(menu);
    auto ensureTrayShown = [tray]() {
        if (!QSystemTrayIcon::isSystemTrayAvailable()) {
            return;
        }
        tray->show();
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
        if (!tray->isVisible()) {
            QTimer::singleShot(1000, tray, [tray]() { tray->show(); });
        }
#endif
    };
    ensureTrayShown();
    QTimer::singleShot(1200, tray, ensureTrayShown);

    QObject::connect(tray, &QSystemTrayIcon::activated, [&](QSystemTrayIcon::ActivationReason reason) {
        if ((reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick)
            && Config::instance().trayClickToOpen) {
            showSidebar();
        }
    });

    updatePos();
    if (Config::instance().startCollapsed) {
        showBall();
    } else {
        showSidebar();
    }

    return app.exec();
}
