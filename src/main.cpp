#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QIcon>
#include <QMenu>
#include <QScreen>
#include <QStyle>
#include <QSystemTrayIcon>

#include "Utils.h"
#include "ui/FloatingBall.h"
#include "ui/Sidebar.h"
#include "ui/Tools.h"

namespace {
constexpr int kSidebarWidth = 84;
}

int main(int argc, char* argv[]) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    auto applyTheme = [&]() {
        if (Config::instance().darkMode) {
            app.setStyleSheet(
                "QWidget{background:#1f2329;color:#e6edf3;}"
                "QPushButton{background:#30363d;border:1px solid #586069;padding:6px;border-radius:8px;color:#e6edf3;}"
                "QLineEdit,QListWidget,QTableWidget,QComboBox{background:#0d1117;color:#e6edf3;border:1px solid #586069;}"
                "QLabel{color:#e6edf3;}");
        } else {
            app.setStyleSheet("");
        }
    };

    if (!Config::instance().firstRunCompleted) {
        FirstRunWizard wizard;
        wizard.exec();
    }
    applyTheme();

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
        applyTheme();
        sidebar->reloadConfig();
        updatePos();
    };

    QObject::connect(ball, &FloatingBall::clicked, showSidebar);
    QObject::connect(sidebar, &Sidebar::requestHide, showBall);

    auto* tray = new QSystemTrayIcon(&app);
    QIcon trayIcon(":/assets/icon_tray.png");
    if (trayIcon.isNull()) {
        trayIcon = QIcon::fromTheme("applications-education");
    }
    if (trayIcon.isNull()) {
        trayIcon = app.style()->standardIcon(QStyle::SP_ComputerIcon);
    }
    tray->setIcon(trayIcon);
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
    tray->show();

    QObject::connect(tray, &QSystemTrayIcon::activated, [&](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger && Config::instance().trayClickToOpen) {
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
