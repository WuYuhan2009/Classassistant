#include "Utils.h"

#include <QDir>
#include <QFile>
#include <QTextStream>

namespace {
QVector<AppButton> defaultButtons() {
    return {
        {"希沃白板", ":/assets/seewo.png", "exe", "SEEWO", true},
        {"班级考勤", ":/assets/check.png", "func", "ATTENDANCE", true},
        {"快速换课", ":/assets/classisland.png", "url", "classisland://open", true},
        {"随机点名", ":/assets/dice.png", "func", "RANDOM_CALL", true},
        {"AI 助手", ":/assets/ai.png", "url", "https://www.doubao.com/chat/", true},
        {"系统设置", ":/assets/settings.png", "func", "SETTINGS", true},
    };
}

QStringList defaultStudents() {
    return {"张三", "李四", "王五", "赵六", "示例学生"};
}

void applyDefaults(QString& seewoPath, QVector<AppButton>& buttons, QStringList& students) {
    seewoPath = "C:/Program Files (x86)/Seewo/EasiNote5/swenlauncher/swenlauncher.exe";
    buttons = defaultButtons();
    students = defaultStudents();
}
}  // namespace

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
        applyDefaults(seewoPath, m_buttons, m_students);
        save();
        return;
    }

    const QByteArray data = file.readAll();
    const auto doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        applyDefaults(seewoPath, m_buttons, m_students);
        save();
        return;
    }

    const QJsonObject root = doc.object();
    seewoPath = root["seewoPath"].toString().trimmed();

    m_students.clear();
    const auto students = root["students"].toArray();
    for (const auto& v : students) {
        const QString name = v.toString().trimmed();
        if (!name.isEmpty()) {
            m_students.append(name);
        }
    }

    m_buttons.clear();
    const QJsonArray btnArr = root["buttons"].toArray();
    for (const auto& v : btnArr) {
        const QJsonObject o = v.toObject();
        const QString name = o["name"].toString().trimmed();
        const QString action = o["action"].toString().trimmed();
        if (name.isEmpty() || action.isEmpty()) {
            continue;
        }

        m_buttons.append({name,
                          o["icon"].toString().trimmed(),
                          action,
                          o["target"].toString().trimmed(),
                          o["isSystem"].toBool()});
    }

    if (seewoPath.isEmpty()) {
        seewoPath = "C:/Program Files (x86)/Seewo/EasiNote5/swenlauncher/swenlauncher.exe";
    }
    if (m_students.isEmpty()) {
        m_students = defaultStudents();
    }
    if (m_buttons.isEmpty()) {
        m_buttons = defaultButtons();
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
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
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
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QStringList parsedStudents;
    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) {
            continue;
        }

        line.replace(';', ',');
        const QStringList parts = line.split(',', Qt::SkipEmptyParts);
        if (parts.size() > 1) {
            for (const auto& part : parts) {
                const QString name = part.trimmed();
                if (!name.isEmpty()) {
                    parsedStudents.append(name);
                }
            }
        } else {
            parsedStudents.append(line);
        }
    }

    parsedStudents.removeDuplicates();
    if (!parsedStudents.isEmpty()) {
        m_students = parsedStudents;
        save();
    }
}
