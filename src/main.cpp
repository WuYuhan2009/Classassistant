#include <QAction>
#include <QApplication>
#include <QCoreApplication>
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
}

int main(int argc, char* argv[]) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    QFont uiFont("HarmonyOS Sans SC");
    if (!uiFont.exactMatch()) {
        uiFont = QFont("HarmonyOS Sans");
    }
    uiFont.setPointSize(10);
    app.setFont(uiFont);
    app.setStyleSheet("QWidget{font-family:'HarmonyOS Sans SC','HarmonyOS Sans','Microsoft YaHei',sans-serif;}"
                      "QToolTip{background:#ffffff;color:#1f2d3d;border:1px solid #d8e0eb;padding:6px;border-radius:8px;}");

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
        ball->moveToBottomRight();
        ball->show();
    } else {
        showSidebar();
    }

    return app.exec();
}
