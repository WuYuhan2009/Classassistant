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

    QVector<AppButton> getButtons() const;
    void setButtons(const QVector<AppButton>& btns);

    QStringList getStudentList() const;
    void setStudentList(const QStringList& list);
    bool importStudentsFromText(const QString& filePath, QString* errorMessage = nullptr);

    bool darkMode = false;
    int sidebarWidth = 70;
    int iconSize = 48;
    bool firstRunCompleted = false;
    QString seewoPath;

private:
    Config();

    QString m_configPath;
    QVector<AppButton> m_buttons;
    QStringList m_students;
};
