#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QCursor>
#include <QFont>
#include <QIcon>
#include <QMenu>
#include <QScreen>
#include <QStyle>
#include <QSystemTrayIcon>

#include "Utils.h"
#include "ui/FloatingBall.h"
#include "ui/FluentTheme.h"
#include "ui/Sidebar.h"
#include "ui/Tools.h"

namespace {
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
    app.setApplicationVersion("2.0.0");

    QFont uiFont("HarmonyOS Sans SC");
    if (!uiFont.exactMatch()) {
        uiFont = QFont("HarmonyOS Sans");
    }
    uiFont.setPointSize(10);
    app.setFont(uiFont);
    app.setStyleSheet("QWidget{font-family:'HarmonyOS Sans SC','HarmonyOS Sans','Microsoft YaHei',sans-serif;}");

    Logger::instance().info("程序启动");

    if (!Config::instance().firstRunCompleted) {
        FirstRunWizard wizard;
        wizard.exec();
    }

    auto* sidebar = new Sidebar();
    auto* ball = new FloatingBall();

    auto showMenu = [&]() {
        sidebar->setAnchorGeometry(ball->geometry());
        sidebar->expandMenu();
        Logger::instance().info("悬浮球点击展开");
    };

    auto showBall = [&]() {
        sidebar->hide();
        ball->setWindowOpacity(Config::instance().floatingOpacity / 100.0);
        ball->show();
    };

    QObject::connect(ball, &FloatingBall::clicked, showMenu);
    QObject::connect(sidebar, &Sidebar::requestCollapseToBall, showBall);
    QObject::connect(ball, &FloatingBall::positionCommitted, [](const QPoint& pt) {
        auto& cfg = Config::instance();
        cfg.floatingBallX = pt.x();
        cfg.floatingBallY = pt.y();
        cfg.save();
    });

    QObject::connect(&app, &QCoreApplication::aboutToQuit, [&]() {
        AppState::setQuitting(true);
        Logger::instance().info("程序退出");
    });

    auto* tray = new QSystemTrayIcon(&app);
    QIcon trayIcon = loadNamedIcon("icon_tray.png");
    if (trayIcon.isNull()) {
        trayIcon = loadNamedIcon("icon_settings.png");
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
    menu->setAttribute(Qt::WA_AcceptTouchEvents);
    menu->setWindowFlag(Qt::FramelessWindowHint);
    menu->setAttribute(Qt::WA_TranslucentBackground);
    menu->setStyleSheet(FluentTheme::trayMenuStyle());

    auto* actionShowMenu = menu->addAction("展开悬浮菜单");
    auto* actionHideMenu = menu->addAction("收起悬浮菜单");
    menu->addSeparator();
    auto* actionAttendance = menu->addAction("快速打开：考勤");
    auto* actionRandomCall = menu->addAction("快速打开：随机点名");
    auto* actionAiAssistant = menu->addAction("快速打开：AI 助手");
    auto* actionOpenSettings = menu->addAction("打开设置");
    menu->addSeparator();
    auto* actionQuit = menu->addAction("退出程序");

    QObject::connect(actionShowMenu, &QAction::triggered, showMenu);
    QObject::connect(actionHideMenu, &QAction::triggered, [&]() { sidebar->collapseMenu(); });
    QObject::connect(actionAttendance, &QAction::triggered, [sidebar]() { sidebar->triggerTool("ATTENDANCE"); });
    QObject::connect(actionRandomCall, &QAction::triggered, [sidebar]() { sidebar->triggerTool("RANDOM_CALL"); });
    QObject::connect(actionAiAssistant, &QAction::triggered, [sidebar]() { sidebar->triggerTool("AI_ASSISTANT"); });
    QObject::connect(actionOpenSettings, &QAction::triggered, [sidebar]() { sidebar->openSettings(); });
    QObject::connect(actionQuit, &QAction::triggered, [&]() {
        AppState::setQuitting(true);
        app.quit();
    });

    tray->show();

    QObject::connect(tray, &QSystemTrayIcon::activated, [&](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Context) {
            menu->popup(QCursor::pos());
            return;
        }
        if ((reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick)
            && Config::instance().trayClickToOpen) {
            showMenu();
        }
    });

    ball->restoreSavedPosition();
    if (Config::instance().startCollapsed) {
        ball->show();
    } else {
        showMenu();
    }

    return app.exec();
}
