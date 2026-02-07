#include "Utils.h"

#include <QDir>
#include <QFile>

Config& Config::instance() {
    static Config ins;
    return ins;
}

Config::Config() {
    const QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataPath);
    m_configPath = dataPath + "/config.json";
    load();
}

void Config::load() {
    QFile file(m_configPath);
    if (!file.open(QIODevice::ReadOnly)) {
        seewoPath = "C:/Program Files (x86)/Seewo/EasiNote5/swenlauncher/swenlauncher.exe";
        m_students = {"张三", "李四", "王五", "赵六", "示例学生"};

        m_buttons = {
            {"希沃白板", ":/assets/seewo.png", "exe", "SEEWO", true},
            {"班级考勤", ":/assets/check.png", "func", "ATTENDANCE", true},
            {"快速换课", ":/assets/classisland.png", "url", "classisland://open", true},
            {"随机点名", ":/assets/dice.png", "func", "RANDOM_CALL", true},
            {"AI 助手", ":/assets/ai.png", "url", "https://www.doubao.com/chat/", true},
            {"系统设置", ":/assets/settings.png", "func", "SETTINGS", true},
        };
        save();
        return;
    }

    const QByteArray data = file.readAll();
    const QJsonObject root = QJsonDocument::fromJson(data).object();
    seewoPath = root["seewoPath"].toString();

    m_students.clear();
    for (const auto& v : root["students"].toArray()) {
        m_students.append(v.toString());
    }

    m_buttons.clear();
    const QJsonArray btnArr = root["buttons"].toArray();
    for (const auto& v : btnArr) {
        const QJsonObject o = v.toObject();
        m_buttons.append({o["name"].toString(),
                          o["icon"].toString(),
                          o["action"].toString(),
                          o["target"].toString(),
                          o["isSystem"].toBool()});
    }
}

void Config::save() {
    QJsonObject root;
    root["seewoPath"] = seewoPath;

    QJsonArray stuArr;
    for (const auto& s : m_students) {
        stuArr.append(s);
    }
    root["students"] = stuArr;

    QJsonArray btnArr;
    for (const auto& b : m_buttons) {
        QJsonObject o;
        o["name"] = b.name;
        o["icon"] = b.iconPath;
        o["action"] = b.action;
        o["target"] = b.target;
        o["isSystem"] = b.isSystem;
        btnArr.append(o);
    }
    root["buttons"] = btnArr;

    QFile file(m_configPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(root).toJson());
    }
}

QVector<AppButton> Config::getButtons() {
    return m_buttons;
}

void Config::setButtons(const QVector<AppButton>& btns) {
    m_buttons = btns;
    save();
}

QStringList Config::getStudentList() {
    return m_students;
}

void Config::setStudentList(const QStringList& list) {
    m_students = list;
    save();
}

void Config::importStudentsFromText(const QString& filePath) {
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_students.clear();
        while (!file.atEnd()) {
            const QString line = QString::fromUtf8(file.readLine()).trimmed();
            if (!line.isEmpty()) {
                m_students.append(line);
            }
        }
        save();
    }
}
