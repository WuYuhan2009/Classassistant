#pragma once

#include <QObject>
#include <QSystemTrayIcon>

class SidebarWindow;
class FloatingBallWindow;
class AttendanceOverlay;
class ConfigManager;
class ResourceDownloader;

class AppController : public QObject {
    Q_OBJECT
public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController() override;

    void start();

public slots:
    void showSidebar();
    void hideSidebar();
    void reloadConfig();

private slots:
    void showSettings();
    void quitApplication();
    void onSidebarCollapseRequested();
    void onFloatingBallClicked();

private:
    void setupTray();
    void applyTheme();

    ConfigManager *m_configManager;
    ResourceDownloader *m_downloader;
    SidebarWindow *m_sidebar;
    FloatingBallWindow *m_ball;
    AttendanceOverlay *m_overlay;
    QSystemTrayIcon *m_trayIcon;
};
