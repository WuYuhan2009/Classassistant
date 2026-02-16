#pragma once

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QVector>

struct AppButton {
    QString name;
    QString iconPath;
    QString action;
    QString target;
    bool isSystem;
};

class Config {
public:
    static Config& instance();

    void load();
    void save();
    void resetToDefaults(bool preserveFirstRun = false);

    QVector<AppButton> getButtons() const;
    void setButtons(const QVector<AppButton>& btns);

    QStringList getStudentList() const;
    void setStudentList(const QStringList& list);
    bool importStudentsFromText(const QString& filePath, QString* errorMessage = nullptr);

    QString resolveIconPath(const QString& iconRef) const;

    int iconSize = 46;
    int floatingOpacity = 85;
    int attendanceSummaryWidth = 360;
    bool startCollapsed = false;
    bool trayClickToOpen = true;
    bool showAttendanceSummaryOnStart = true;
    bool randomNoRepeat = true;
    bool allowExternalLinks = false;
    bool compactMode = false;
    int randomHistorySize = 5;
    int animationDurationMs = 240;
    int sidebarWidth = 92;
    int groupSplitSize = 4;
    QString scoreTeamAName = "红队";
    QString scoreTeamBName = "蓝队";
    bool collapseHidesToolWindows = true;
    bool firstRunCompleted = false;
    QString seewoPath;
    QString classNote;

private:
    Config();

    QString m_configPath;
    QVector<AppButton> m_buttons;
    QStringList m_students;
};
