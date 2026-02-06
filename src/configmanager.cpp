#include "configmanager.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QStandardPaths>

ConfigManager& ConfigManager::instance() {
    static ConfigManager instance;
    return instance;
}

ConfigManager::ConfigManager() {
    ensureConfigDir();
    const QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_configFilePath = configDir + "/config.json";
    if (!load()) {
        loadDefaults();
        save();
    }
}

ConfigManager::~ConfigManager() { save(); }

void ConfigManager::ensureConfigDir() {
    QDir dir;
    const QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!dir.exists(configDir)) dir.mkpath(configDir);
}

void ConfigManager::loadDefaults() {
    m_config = QJsonObject();
    m_config["theme"] = "light";
    m_config["sidebarWidth"] = 220;
    m_config["iconSize"] = 48;
    m_config["seewoPath"] = "";

    QJsonArray defaultButtons;
    QStringList buttonNames = {"希沃白板", "班级考勤", "ClassIsland", "随机点名", "AI助手", "设置"};
    QStringList buttonIcons = {"seewo", "attendance", "classisland", "random", "ai", "settings"};
    QStringList buttonTypes = {"program", "attendance", "classisland", "random", "url", "settings"};
    QStringList buttonData = {"", "", "", "", "https://www.doubao.com/chat/", ""};

    for (int i = 0; i < buttonNames.size(); ++i) {
        QJsonObject btn;
        btn["name"] = buttonNames[i];
        btn["icon"] = buttonIcons[i];
        btn["type"] = buttonTypes[i];
        btn["data"] = buttonData[i];
        btn["enabled"] = true;
        btn["isDefault"] = true;
        defaultButtons.append(btn);
    }
    m_config["buttons"] = defaultButtons;
    m_config["students"] = QJsonArray();
}

bool ConfigManager::save() {
    QFile file(m_configFilePath);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(QJsonDocument(m_config).toJson(QJsonDocument::Indented));
    emit configChanged();
    return true;
}

bool ConfigManager::load() {
    QFile file(m_configFilePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) return false;
    const auto doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) return false;
    m_config = doc.object();
    return true;
}

QString ConfigManager::configPath() const { return m_configFilePath; }
bool ConfigManager::isDarkMode() const { return m_config["theme"].toString() == "dark"; }
void ConfigManager::setDarkMode(bool dark) { m_config["theme"] = dark ? "dark" : "light"; }
int ConfigManager::sidebarWidth() const { return m_config["sidebarWidth"].toInt(220); }
void ConfigManager::setSidebarWidth(int width) { m_config["sidebarWidth"] = width; }
int ConfigManager::iconSize() const { return m_config["iconSize"].toInt(48); }
void ConfigManager::setIconSize(int size) { m_config["iconSize"] = size; }

QList<ButtonConfig> ConfigManager::getButtons() const {
    QList<ButtonConfig> buttons;
    const QJsonArray arr = m_config["buttons"].toArray();
    for (const auto &v : arr) {
        const auto o = v.toObject();
        ButtonConfig b;
        b.name = o["name"].toString();
        b.icon = o["icon"].toString();
        b.type = o["type"].toString();
        b.data = o["data"].toString();
        b.enabled = o["enabled"].toBool(true);
        b.isDefault = o["isDefault"].toBool(false);
        buttons.append(b);
    }
    return buttons;
}

void ConfigManager::setButtons(const QList<ButtonConfig> &buttons) {
    QJsonArray arr;
    for (const auto &b : buttons) {
        QJsonObject o;
        o["name"] = b.name;
        o["icon"] = b.icon;
        o["type"] = b.type;
        o["data"] = b.data;
        o["enabled"] = b.enabled;
        o["isDefault"] = b.isDefault;
        arr.append(o);
    }
    m_config["buttons"] = arr;
}

QString ConfigManager::getSeewoPath() const { return m_config["seewoPath"].toString(); }
void ConfigManager::setSeewoPath(const QString &path) { m_config["seewoPath"] = path; }
QStringList ConfigManager::getStudents() const {
    QStringList out;
    for (const auto &v : m_config["students"].toArray()) out << v.toString();
    return out;
}
void ConfigManager::setStudents(const QStringList &students) {
    QJsonArray arr;
    for (const auto &s : students) arr.append(s);
    m_config["students"] = arr;
}
