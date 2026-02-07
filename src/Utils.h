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

    QVector<AppButton> getButtons();
    void setButtons(const QVector<AppButton>& btns);

    QStringList getStudentList();
    void setStudentList(const QStringList& list);
    void importStudentsFromText(const QString& filePath);

    QString seewoPath;

private:
    Config();

    QString m_configPath;
    QVector<AppButton> m_buttons;
    QStringList m_students;
};
