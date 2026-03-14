#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QCursor>
#include <QDateTime>
#include <QEvent>
#include <QFont>
#include <QIcon>
#include <QMenu>
#include <QScreen>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QTimer>

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
    app.setApplicationName("ClassFlow");
    app.setApplicationDisplayName("ClassFlow");
    app.setApplicationVersion("2.0.1 Beta");

    QFont uiFont("HarmonyOS Sans SC");
    if (!uiFont.exactMatch()) uiFont = QFont("HarmonyOS Sans");
    uiFont.setPointSize(10);
    app.setFont(uiFont);
    app.setStyleSheet("QWidget{font-family:'HarmonyOS Sans SC','HarmonyOS Sans','Microsoft YaHei',sans-serif;}"
                      "QToolTip{background:rgba(32,45,68,220);color:#f2f7ff;border:1px solid #7b9fcc;padding:6px;border-radius:8px;}");

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

    QObject::connect(ball, &FloatingBall::clicked, [&]() {
        if (sidebar->isExpanded()) sidebar->collapseMenu();
        else showMenu();
    });
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
    QIcon trayIcon = loadNamedIcon("icon_tray.svg");
    if (trayIcon.isNull()) trayIcon = loadNamedIcon("icon_settings.svg");
    if (trayIcon.isNull()) trayIcon = QIcon::fromTheme("applications-education");
    if (trayIcon.isNull()) trayIcon = app.style()->standardIcon(QStyle::SP_ComputerIcon);
    tray->setIcon(trayIcon);
    app.setWindowIcon(trayIcon);
    tray->setToolTip("ClassFlow");

    auto* menu = new QMenu();
    menu->setAttribute(Qt::WA_AcceptTouchEvents);
    menu->setWindowFlag(Qt::FramelessWindowHint);
    menu->setAttribute(Qt::WA_TranslucentBackground);
    menu->setStyleSheet(FluentTheme::trayMenuStyle());

    auto* actionShowMenu = menu->addAction("展开悬浮菜单");
    auto* actionHideMenu = menu->addAction("收起悬浮菜单");
    menu->addSeparator();
    auto* actionAttendance = menu->addAction("快速打开：考勤");
    auto* actionScreenOff = menu->addAction("快速打开：息屏");
    auto* actionRandomCall = menu->addAction("快速打开：随机点名");
    auto* actionAiAssistant = menu->addAction("快速打开：AI 助手");
    auto* actionOpenSettings = menu->addAction("打开设置");
    menu->addSeparator();
    auto* actionQuit = menu->addAction("退出程序");

    QObject::connect(actionShowMenu, &QAction::triggered, showMenu);
    QObject::connect(actionHideMenu, &QAction::triggered, [&]() { sidebar->collapseMenu(); });
    QObject::connect(actionAttendance, &QAction::triggered, [sidebar]() { sidebar->triggerTool("ATTENDANCE"); });
    QObject::connect(actionScreenOff, &QAction::triggered, [sidebar]() { sidebar->triggerTool("SCREEN_OFF"); });
    QObject::connect(actionRandomCall, &QAction::triggered, [sidebar]() { sidebar->triggerTool("RANDOM_CALL"); });
    QObject::connect(actionAiAssistant, &QAction::triggered, [sidebar]() { sidebar->triggerTool("AI_ASSISTANT"); });
    QObject::connect(actionOpenSettings, &QAction::triggered, [sidebar]() { sidebar->openSettings(); });
    QObject::connect(actionQuit, &QAction::triggered, [&]() { AppState::setQuitting(true); app.quit(); });

    tray->show();
    QObject::connect(tray, &QSystemTrayIcon::activated, [&](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Context) {
            menu->popup(QCursor::pos());
            return;
        }
        if ((reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) && Config::instance().trayClickToOpen) {
            showMenu();
        }
    });

    QString triggeredStudyKey;
    QTimer studyTimer;
    studyTimer.setInterval(10000);
    QObject::connect(&studyTimer, &QTimer::timeout, [&]() {
        const QTime now = QTime::currentTime();
        QString currentKey;
        for (const QString& period : Config::instance().selfStudyPeriods) {
            const QStringList parts = period.split('-', Qt::SkipEmptyParts);
            if (parts.size() != 2) continue;
            const QTime s = QTime::fromString(parts[0].trimmed(), "HH:mm");
            const QTime e = QTime::fromString(parts[1].trimmed(), "HH:mm");
            if (!s.isValid() || !e.isValid()) continue;
            if (now >= s && now <= e) {
                currentKey = QDate::currentDate().toString(Qt::ISODate) + "|" + period;
                break;
            }
        }

        if (currentKey.isEmpty()) {
            triggeredStudyKey.clear();
            return;
        }

        if (triggeredStudyKey != currentKey) {
            triggeredStudyKey = currentKey;
            sidebar->triggerTool("SCREEN_OFF");
            Logger::instance().info("根据自习时段自动进入息屏");
        }
    });
    studyTimer.start();

    ball->restoreSavedPosition();
    if (Config::instance().startCollapsed) ball->show();
    else showMenu();

    return app.exec();
}
