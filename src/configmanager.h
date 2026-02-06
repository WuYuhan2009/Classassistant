#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <QStringList>
#include "buttonconfig.h"

class ConfigManager : public QObject
{
    Q_OBJECT
public:
    static ConfigManager& instance();

    bool isDarkMode() const;
    void setDarkMode(bool dark);

    int sidebarWidth() const;
    void setSidebarWidth(int width);

    int iconSize() const;
    void setIconSize(int size);

    QList<ButtonConfig> getButtons() const;
    void setButtons(const QList<ButtonConfig> &buttons);

    QString getSeewoPath() const;
    void setSeewoPath(const QString &path);

    QStringList getStudents() const;
    void setStudents(const QStringList &students);

    bool save();
    bool load();
    QString configPath() const;

signals:
    void configChanged();

private:
    ConfigManager();
    ~ConfigManager();
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    void loadDefaults();
    void ensureConfigDir();

    QJsonObject m_config;
    QString m_configFilePath;
};

#endif
