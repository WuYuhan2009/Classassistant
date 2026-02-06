#include "configmanager.h"

#include <QDate>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

ConfigManager::ConfigManager(QObject *parent) : QObject(parent) {
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(base);
    m_configFile = base + "/config.json";
    loadDefaults();
}

void ConfigManager::loadDefaults() {
    m_config = AppConfig{};
    ensureDefaultButtons();
}

void ConfigManager::ensureDefaultButtons() {
    if (!m_config.buttons.isEmpty()) {
        return;
    }
    m_config.buttons = {
        {"default_whiteboard", "希沃白板", "assets/icons/whiteboard.png", "", "internal", false},
        {"default_attendance", "班级考勤", "assets/icons/attendance.png", "", "internal", false},
        {"default_classisland", "快捷换课", "assets/icons/classisland.png", "", "internal", false},
        {"default_random", "随机点名", "assets/icons/random.png", "", "internal", false},
        {"default_ai", "AI", "assets/icons/ai.png", "https://www.doubao.com/chat/", "url", false},
        {"default_settings", "设置", "assets/icons/settings.png", "", "internal", false}
    };
}

bool ConfigManager::load() {
    QFile file(m_configFile);
    if (!file.exists()) {
        loadDefaults();
        save();
        return true;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    const QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
    m_config.firstRunCompleted = root.value("firstRunCompleted").toBool(false);

    const QJsonObject theme = root.value("theme").toObject();
    m_config.theme.darkMode = theme.value("darkMode").toBool(true);
    m_config.theme.sidebarWidth = theme.value("sidebarWidth").toInt(84);
    m_config.theme.iconSize = theme.value("iconSize").toInt(30);

    m_config.whiteboardPath = root.value("whiteboardPath").toString();

    m_config.buttons.clear();
    const QJsonArray buttons = root.value("buttons").toArray();
    for (const QJsonValue &v : buttons) {
        const QJsonObject o = v.toObject();
        m_config.buttons.push_back(ButtonAction{
            o.value("id").toString(),
            o.value("title").toString(),
            o.value("iconPath").toString(),
            o.value("target").toString(),
            o.value("actionType").toString(),
            o.value("removable").toBool(true)
        });
    }
    if (m_config.buttons.isEmpty()) {
        ensureDefaultButtons();
    }

    const QJsonObject att = root.value("attendance").toObject();
    m_config.attendance.lastDate = att.value("lastDate").toString();
    for (const QJsonValue &v : att.value("allStudents").toArray()) {
        m_config.attendance.allStudents.push_back(v.toString());
    }
    for (const QJsonValue &v : att.value("absentStudents").toArray()) {
        m_config.attendance.absentStudents.push_back(v.toString());
    }

    return true;
}

bool ConfigManager::save() const {
    QJsonObject root;
    root.insert("firstRunCompleted", m_config.firstRunCompleted);

    QJsonObject theme;
    theme.insert("darkMode", m_config.theme.darkMode);
    theme.insert("sidebarWidth", m_config.theme.sidebarWidth);
    theme.insert("iconSize", m_config.theme.iconSize);
    root.insert("theme", theme);
    root.insert("whiteboardPath", m_config.whiteboardPath);

    QJsonArray buttons;
    for (const auto &b : m_config.buttons) {
        QJsonObject o;
        o.insert("id", b.id);
        o.insert("title", b.title);
        o.insert("iconPath", b.iconPath);
        o.insert("target", b.target);
        o.insert("actionType", b.actionType);
        o.insert("removable", b.removable);
        buttons.append(o);
    }
    root.insert("buttons", buttons);

    QJsonObject att;
    att.insert("lastDate", m_config.attendance.lastDate);
    QJsonArray all;
    for (const auto &s : m_config.attendance.allStudents) all.append(s);
    QJsonArray absent;
    for (const auto &s : m_config.attendance.absentStudents) absent.append(s);
    att.insert("allStudents", all);
    att.insert("absentStudents", absent);
    root.insert("attendance", att);

    QFile file(m_configFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

void ConfigManager::resetDailyAttendanceIfNeeded() {
    const QString today = QDate::currentDate().toString(Qt::ISODate);
    if (m_config.attendance.lastDate != today) {
        m_config.attendance.lastDate = today;
        m_config.attendance.absentStudents.clear();
        save();
    }
}

AppConfig &ConfigManager::config() { return m_config; }
const AppConfig &ConfigManager::config() const { return m_config; }
QString ConfigManager::configFilePath() const { return m_configFile; }
