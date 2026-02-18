#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QCursor>
#include <QFont>
#include <QIcon>
#include <QMenu>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QRect>
#include <QScreen>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QTimer>

#include "Utils.h"
#include "ui/FloatingBall.h"
#include "ui/Sidebar.h"
#include "ui/Tools.h"

namespace {
QIcon loadNamedIcon(const QString& fileName) {
    const QString resolvedPath = Config::instance().resolveIconPath(fileName);
    return QIcon(resolvedPath);
}

QString buildMaterialStyleSheet() {
    return "QWidget{font-family:'HarmonyOS Sans SC','HarmonyOS Sans','Microsoft YaHei',sans-serif;color:#1d1b20;background:transparent;}"
           "QAbstractButton{min-height:40px;padding:8px 12px;font-size:14px;border-radius:20px;border:1px solid #7a757f;background:#fffbfe;color:#1d1b20;}"
           "QAbstractButton:hover{background:#f5f0fa;}"
           "QAbstractButton:pressed{background:#ece6f0;}"
           "QLineEdit,QTextEdit,QPlainTextEdit,QListWidget,QTreeWidget,QComboBox,QSpinBox,QTableWidget{"
           "background:#fffbfe;border:1px solid #79747e;border-radius:12px;padding:8px;selection-background-color:#e8def8;selection-color:#1d192b;}"
           "QToolTip{background:#313033;color:#f4eff4;border:1px solid #49454f;padding:8px;border-radius:8px;}"
           "QScrollBar:vertical{width:10px;background:transparent;margin:2px;}"
           "QScrollBar::handle:vertical{background:#cac4d0;min-height:20px;border-radius:5px;}"
           "QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical{height:0;}"
           "QMenu{background:#fef7ff;border:1px solid #e7e0ec;border-radius:16px;padding:8px;}"
           "QMenu::item{font-size:14px;color:#1d1b20;padding:10px 14px;border-radius:12px;margin:2px 4px;min-height:34px;}"
           "QMenu::item:selected{background:#ece6f0;color:#4a4458;}"
           "QMenu::separator{height:1px;background:#e7e0ec;margin:8px 6px;}";
}
}

int main(int argc, char* argv[]) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QCoreApplication::setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents);
    QCoreApplication::setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents);

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    app.setApplicationVersion("1.0.0");

    QFont uiFont("HarmonyOS Sans SC");
    if (!uiFont.exactMatch()) {
        uiFont = QFont("HarmonyOS Sans");
    }
    uiFont.setPointSize(10);
    app.setFont(uiFont);
    app.setStyleSheet(buildMaterialStyleSheet());

    if (!Config::instance().firstRunCompleted) {
        FirstRunWizard wizard;
        wizard.exec();
    }

    auto* sidebar = new Sidebar();
    auto* ball = new FloatingBall();

    auto updatePos = [&]() {
        const QRect screen = app.primaryScreen()->availableGeometry();
        const int sidebarWidth = qMax(84, Config::instance().sidebarWidth);
        sidebar->resize(sidebarWidth, qMin(screen.height() - 20, 800));
        sidebar->move(screen.right() - sidebarWidth + 1, screen.center().y() - sidebar->height() / 2);
        ball->setWindowOpacity(Config::instance().floatingOpacity / 100.0);
    };

    auto animateSidebarIn = [&]() {
        updatePos();
        sidebar->show();
        sidebar->raise();

        const QRect endRect = sidebar->geometry();
        const QRect startRect(endRect.x() + 26, endRect.y(), endRect.width(), endRect.height());
        sidebar->setGeometry(startRect);
        sidebar->setWindowOpacity(0.0);

        auto* group = new QParallelAnimationGroup(sidebar);
        auto* moveAnim = new QPropertyAnimation(sidebar, "geometry", group);
        moveAnim->setDuration(Config::instance().animationDurationMs);
        moveAnim->setStartValue(startRect);
        moveAnim->setEndValue(endRect);
        moveAnim->setEasingCurve(QEasingCurve::OutBack);

        auto* opacityAnim = new QPropertyAnimation(sidebar, "windowOpacity", group);
        opacityAnim->setDuration(Config::instance().animationDurationMs);
        opacityAnim->setStartValue(0.0);
        opacityAnim->setEndValue(1.0);
        opacityAnim->setEasingCurve(QEasingCurve::OutQuint);

        group->addAnimation(moveAnim);
        group->addAnimation(opacityAnim);
        group->start(QAbstractAnimation::DeleteWhenStopped);
    };

    auto showSidebar = [&]() {
        ball->hide();
        animateSidebarIn();
        sidebar->activateWindow();
    };

    auto showBall = [&]() {
        sidebar->hideAllToolWindowsAnimated();

        const QRect rect = sidebar->geometry();
        auto* group = new QParallelAnimationGroup(sidebar);
        auto* moveAnim = new QPropertyAnimation(sidebar, "geometry", group);
        moveAnim->setDuration(Config::instance().animationDurationMs);
        moveAnim->setStartValue(rect);
        moveAnim->setEndValue(QRect(rect.x() + 26, rect.y(), rect.width(), rect.height()));
        moveAnim->setEasingCurve(QEasingCurve::InCubic);

        auto* opacityAnim = new QPropertyAnimation(sidebar, "windowOpacity", group);
        opacityAnim->setDuration(Config::instance().animationDurationMs);
        opacityAnim->setStartValue(sidebar->windowOpacity());
        opacityAnim->setEndValue(0.0);
        opacityAnim->setEasingCurve(QEasingCurve::InCubic);

        group->addAnimation(moveAnim);
        group->addAnimation(opacityAnim);

        QObject::connect(group, &QParallelAnimationGroup::finished, sidebar, [=]() {
            sidebar->hide();
            sidebar->setWindowOpacity(1.0);
            updatePos();
            ball->moveToBottomRight();
            ball->show();

            auto* ballAnim = new QPropertyAnimation(ball, "windowOpacity", ball);
            ballAnim->setDuration(Config::instance().animationDurationMs);
            ball->setWindowOpacity(0.0);
            ballAnim->setStartValue(0.0);
            ballAnim->setEndValue(Config::instance().floatingOpacity / 100.0);
            ballAnim->setEasingCurve(QEasingCurve::OutBack);
            ballAnim->start(QAbstractAnimation::DeleteWhenStopped);
        });
        group->start(QAbstractAnimation::DeleteWhenStopped);
    };

    auto reloadAll = [&]() {
        Config::instance().load();
        sidebar->reloadConfig();
        updatePos();
    };

    QObject::connect(ball, &FloatingBall::clicked, showSidebar);
    QObject::connect(sidebar, &Sidebar::requestHide, showBall);

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
    menu->setStyleSheet("QMenu{background:#fef7ff;border:1px solid #e7e0ec;border-radius:16px;padding:8px;}"
                        "QMenu::item{font-size:14px;color:#1d1b20;padding:10px 14px;border-radius:12px;margin:2px 4px;min-height:34px;}"
                        "QMenu::item:selected{background:#ece6f0;color:#4a4458;}"
                        "QMenu::separator{height:1px;background:#e7e0ec;margin:8px 6px;}");

    auto* actionShowSidebar = menu->addAction("展开侧边栏");
    auto* actionShowBall = menu->addAction("收起为悬浮球");
    menu->addSeparator();
    auto* actionAttendance = menu->addAction("快速打开：考勤");
    auto* actionRandomCall = menu->addAction("快速打开：随机点名");
    auto* actionClassTimer = menu->addAction("快速打开：课堂计时");
    auto* actionClassNote = menu->addAction("快速打开：课堂便签");
    auto* actionGroupSplit = menu->addAction("快速打开：分组抽签");
    auto* actionScoreBoard = menu->addAction("快速打开：课堂计分板");
    auto* actionAiAssistant = menu->addAction("快速打开：AI 助手");
    menu->addSeparator();
    auto* actionOpenSettings = menu->addAction("打开设置");
    auto* actionReloadConfig = menu->addAction("重载配置");
    menu->addSeparator();
    auto* actionQuit = menu->addAction("退出程序");

    QObject::connect(actionOpenSettings, &QAction::triggered, [sidebar]() { sidebar->openSettings(); });
    QObject::connect(actionShowSidebar, &QAction::triggered, showSidebar);
    QObject::connect(actionShowBall, &QAction::triggered, showBall);
    QObject::connect(actionAttendance, &QAction::triggered, [sidebar]() { sidebar->triggerTool("ATTENDANCE"); });
    QObject::connect(actionRandomCall, &QAction::triggered, [sidebar]() { sidebar->triggerTool("RANDOM_CALL"); });
    QObject::connect(actionClassTimer, &QAction::triggered, [sidebar]() { sidebar->triggerTool("CLASS_TIMER"); });
    QObject::connect(actionClassNote, &QAction::triggered, [sidebar]() { sidebar->triggerTool("CLASS_NOTE"); });
    QObject::connect(actionGroupSplit, &QAction::triggered, [sidebar]() { sidebar->triggerTool("GROUP_SPLIT"); });
    QObject::connect(actionScoreBoard, &QAction::triggered, [sidebar]() { sidebar->triggerTool("SCORE_BOARD"); });
    QObject::connect(actionAiAssistant, &QAction::triggered, [sidebar]() { sidebar->triggerTool("AI_ASSISTANT"); });
    QObject::connect(actionReloadConfig, &QAction::triggered, reloadAll);
    QObject::connect(actionQuit, &QAction::triggered, [&]() { app.quit(); });

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
        if (reason == QSystemTrayIcon::Context) {
            menu->popup(QCursor::pos());
            return;
        }
        if ((reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick)
            && Config::instance().trayClickToOpen) {
            showSidebar();
        }
    });

    updatePos();
    if (Config::instance().startCollapsed) {
        ball->moveToBottomRight();
        ball->show();
    } else {
        showSidebar();
    }

    return app.exec();
}
