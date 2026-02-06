#pragma once

#include <QWidget>
#include "models.h"

class ConfigManager;
class AttendanceOverlay;

class SidebarWindow : public QWidget {
    Q_OBJECT
public:
    explicit SidebarWindow(ConfigManager *config, AttendanceOverlay *overlay, QWidget *parent = nullptr);

    void rebuildButtons();

signals:
    void collapseRequested();
    void settingsRequested();

private:
    ConfigManager *m_config;
    AttendanceOverlay *m_overlay;
    class QVBoxLayout *m_layout;

    void launchAction(const ButtonAction &action);
    void openAttendanceEditor();
    void randomRollCall();
    void openClassIslandShortcut();
};
