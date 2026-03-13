#pragma once

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
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
    static QVector<AppButton> defaultButtons();

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
    int floatingBallSize = 72;
    int floatingOpacity = 85;
    int attendanceSummaryWidth = 360;
    int radialMenuRadius = 210;
    int menuAutoCollapseSeconds = 15;
    bool startCollapsed = true;
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
    QString siliconFlowApiKey;
    QString siliconFlowModel = "deepseek-ai/DeepSeek-V3.2";
    QString siliconFlowEndpoint = "https://api.siliconflow.cn/v1/chat/completions";
    int floatingBallX = -1;
    int floatingBallY = -1;
    QStringList selfStudyPeriods;
    int selfStudyIdleSeconds = 180;
    bool screenOffShowQuote = true;

private:
    Config();

    QString m_configPath;
    QVector<AppButton> m_buttons;
    QStringList m_students;
};

class Logger {
public:
    static Logger& instance();
    void info(const QString& message);
    void warn(const QString& message);
    void error(const QString& message);
    QString logPath() const;

private:
    Logger();
    void write(const QString& level, const QString& message);

    QString m_logPath;
    mutable QMutex m_mutex;
};

namespace AppState {
void setQuitting(bool quitting);
bool isQuitting();
}
