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

bool isNowInSelfStudy() {
    const QTime now = QTime::currentTime();
    for (const QString& period : Config::instance().selfStudyPeriods) {
        const QStringList parts = period.split('-', Qt::SkipEmptyParts);
        if (parts.size() != 2) continue;
        const QTime s = QTime::fromString(parts[0].trimmed(), "HH:mm");
        const QTime e = QTime::fromString(parts[1].trimmed(), "HH:mm");
        if (!s.isValid() || !e.isValid()) continue;
        if (now >= s && now <= e) return true;
    }
    return false;
}

class IdleEventFilter : public QObject {
public:
    explicit IdleEventFilter(QDateTime* lastInput, bool* autoOpened, QObject* parent = nullptr)
        : QObject(parent), m_lastInput(lastInput), m_autoOpened(autoOpened) {}

protected:
    bool eventFilter(QObject* obj, QEvent* e) override {
        Q_UNUSED(obj);
        switch (e->type()) {
        case QEvent::MouseMove:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::KeyPress:
        case QEvent::TouchBegin:
        case QEvent::TouchUpdate:
            if (m_lastInput) *m_lastInput = QDateTime::currentDateTime();
            if (m_autoOpened) *m_autoOpened = false;
            break;
        default:
            break;
        }
        return false;
    }

private:
    QDateTime* m_lastInput;
    bool* m_autoOpened;
};
}

int main(int argc, char* argv[]) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    app.setApplicationVersion("2.1.0");

    QFont uiFont("HarmonyOS Sans SC");
    if (!uiFont.exactMatch()) uiFont = QFont("HarmonyOS Sans");
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
    QIcon trayIcon = loadNamedIcon("icon_tray.png");
    if (trayIcon.isNull()) trayIcon = loadNamedIcon("icon_settings.png");
    if (trayIcon.isNull()) trayIcon = QIcon::fromTheme("applications-education");
    if (trayIcon.isNull()) trayIcon = app.style()->standardIcon(QStyle::SP_ComputerIcon);
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

    QDateTime lastInput = QDateTime::currentDateTime();
    bool autoOpenedInCurrentIdle = false;
    auto* idleFilter = new IdleEventFilter(&lastInput, &autoOpenedInCurrentIdle, &app);
    app.installEventFilter(idleFilter);

    QTimer idleTimer;
    idleTimer.setInterval(10000);
    QObject::connect(&idleTimer, &QTimer::timeout, [&]() {
        if (!isNowInSelfStudy()) {
            autoOpenedInCurrentIdle = false;
            return;
        }
        const qint64 idleSeconds = lastInput.secsTo(QDateTime::currentDateTime());
        if (idleSeconds >= Config::instance().selfStudyIdleSeconds && !autoOpenedInCurrentIdle) {
            sidebar->triggerTool("SCREEN_OFF");
            autoOpenedInCurrentIdle = true;
            Logger::instance().info("自习课检测到3分钟无操作，自动打开息屏");
        }
    });
    idleTimer.start();

    ball->restoreSavedPosition();
    if (Config::instance().startCollapsed) ball->show();
    else showMenu();

    return app.exec();
}
