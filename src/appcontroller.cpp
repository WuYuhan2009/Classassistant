#include "appcontroller.h"

#include "attendanceoverlay.h"
#include "configmanager.h"
#include "floatingballwindow.h"
#include "resourcedownloader.h"
#include "settingsdialog.h"
#include "sidebarwindow.h"
#include "welcomedialog.h"

#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QMessageBox>

AppController::AppController(QObject *parent)
    : QObject(parent),
      m_configManager(new ConfigManager(this)),
      m_downloader(new ResourceDownloader(this)),
      m_sidebar(nullptr),
      m_ball(nullptr),
      m_overlay(nullptr),
      m_trayIcon(nullptr) {}

AppController::~AppController() = default;

void AppController::start() {
    m_configManager->load();
    m_configManager->resetDailyAttendanceIfNeeded();
    m_downloader->ensureOfflineResources();

    if (!m_configManager->config().firstRunCompleted) {
        WelcomeDialog dialog;
        if (dialog.exec() == QDialog::Accepted) {
            auto &cfg = m_configManager->config();
            cfg.theme.darkMode = dialog.darkMode();
            cfg.theme.sidebarWidth = dialog.sidebarWidth();
            cfg.theme.iconSize = dialog.iconSize();
            cfg.firstRunCompleted = true;
            m_configManager->save();
        }
    }

    applyTheme();

    m_overlay = new AttendanceOverlay(m_configManager);
    m_overlay->show();

    m_sidebar = new SidebarWindow(m_configManager, m_overlay);
    connect(m_sidebar, &SidebarWindow::collapseRequested, this, &AppController::onSidebarCollapseRequested);
    connect(m_sidebar, &SidebarWindow::settingsRequested, this, &AppController::showSettings);

    m_ball = new FloatingBallWindow();
    m_ball->hide();
    connect(m_ball, &FloatingBallWindow::clicked, this, &AppController::onFloatingBallClicked);

    setupTray();
    showSidebar();
}

void AppController::applyTheme() {
    if (m_configManager->config().theme.darkMode) {
        qApp->setStyleSheet("QWidget{background:#0f172a;color:#e2e8f0;} QPushButton{border:none;border-radius:12px;padding:6px;} QPushButton:hover{background:#1e293b;}");
    } else {
        qApp->setStyleSheet("QWidget{background:#f8fafc;color:#0f172a;} QPushButton{border:none;border-radius:12px;padding:6px;} QPushButton:hover{background:#e2e8f0;}");
    }
}

void AppController::setupTray() {
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setToolTip("Classassistant");
    m_trayIcon->setIcon(QIcon("assets/icons/settings.png"));

    auto *menu = new QMenu();
    menu->addAction("打开设置", this, &AppController::showSettings);
    menu->addAction("展开侧边栏", this, &AppController::showSidebar);
    menu->addAction("重载配置", this, &AppController::reloadConfig);
    menu->addSeparator();
    menu->addAction("退出程序", this, &AppController::quitApplication);
    m_trayIcon->setContextMenu(menu);

    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            showSidebar();
        }
    });

    m_trayIcon->show();
}

void AppController::showSidebar() {
    if (!m_sidebar) return;
    m_sidebar->show();
    m_sidebar->raise();
    if (m_ball) m_ball->hide();
}

void AppController::hideSidebar() {
    if (!m_sidebar || !m_ball) return;
    m_sidebar->hide();
    m_ball->show();
    m_ball->raise();
}

void AppController::reloadConfig() {
    m_configManager->load();
    applyTheme();
    if (m_sidebar) m_sidebar->rebuildButtons();
    if (m_overlay) m_overlay->refresh();
}

void AppController::showSettings() {
    SettingsDialog dialog(m_configManager);
    if (dialog.exec() == QDialog::Accepted) {
        m_configManager->save();
        applyTheme();
        if (m_sidebar) m_sidebar->rebuildButtons();
        if (m_overlay) m_overlay->refresh();
    }
}

void AppController::quitApplication() {
    if (m_trayIcon) m_trayIcon->hide();
    qApp->quit();
}

void AppController::onSidebarCollapseRequested() {
    hideSidebar();
}

void AppController::onFloatingBallClicked() {
    showSidebar();
}
