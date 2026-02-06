#pragma once

#include <QWidget>

class ConfigManager;
class QLabel;

class AttendanceOverlay : public QWidget {
    Q_OBJECT
public:
    explicit AttendanceOverlay(ConfigManager *config, QWidget *parent = nullptr);

    void refresh();

private:
    ConfigManager *m_config;
    QLabel *m_summary;
};
