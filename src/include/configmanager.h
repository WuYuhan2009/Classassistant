#pragma once

#include "models.h"
#include <QObject>

class ConfigManager : public QObject {
    Q_OBJECT
public:
    explicit ConfigManager(QObject *parent = nullptr);

    bool load();
    bool save() const;
    void resetDailyAttendanceIfNeeded();

    AppConfig &config();
    const AppConfig &config() const;

    QString configFilePath() const;

    void ensureDefaultButtons();

signals:
    void configChanged();

private:
    QString m_configFile;
    AppConfig m_config;

    void loadDefaults();
};
