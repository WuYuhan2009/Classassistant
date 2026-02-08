#include "Utils.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

namespace {
constexpr int kSidebarWidth = 84;

QVector<AppButton> defaultButtons() {
    return {
        {"希沃白板", ":/assets/icon_seewo.png", "exe", "SEEWO", true},
        {"班级考勤", ":/assets/icon_attendance.png", "func", "ATTENDANCE", true},
        {"随机点名", ":/assets/icon_random.png", "func", "RANDOM_CALL", true},
        {"AI 助手", ":/assets/icon_ai.png", "url", "https://www.doubao.com/chat/", true},
    };
}

QStringList defaultStudents() {
    return QStringList{QStringLiteral("张三"), QStringLiteral("李四"), QStringLiteral("王五"), QStringLiteral("赵六"), QStringLiteral("示例学生")};
}

void applyDefaults(Config& config, QVector<AppButton>& buttons, QStringList& students) {
    config.seewoPath = "C:/Program Files (x86)/Seewo/EasiNote5/swenlauncher/swenlauncher.exe";
    config.iconSize = 46;
    config.floatingOpacity = 85;
    config.attendanceSummaryWidth = 360;
    config.startCollapsed = false;
    config.trayClickToOpen = true;
    config.showAttendanceSummaryOnStart = true;
    config.firstRunCompleted = false;
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
        applyDefaults(*this, m_buttons, m_students);
        save();
        return;
    }

    const auto doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        applyDefaults(*this, m_buttons, m_students);
        save();
        return;
    }

    const QJsonObject root = doc.object();
    seewoPath = root["seewoPath"].toString().trimmed();
    iconSize = qBound(28, root["iconSize"].toInt(46), 72);
    floatingOpacity = qBound(35, root["floatingOpacity"].toInt(85), 100);
    attendanceSummaryWidth = qBound(300, root["attendanceSummaryWidth"].toInt(360), 520);
    startCollapsed = root["startCollapsed"].toBool(false);
    trayClickToOpen = root["trayClickToOpen"].toBool(true);
    showAttendanceSummaryOnStart = root["showAttendanceSummaryOnStart"].toBool(true);
    firstRunCompleted = root["firstRunCompleted"].toBool(false);

    m_students.clear();
    for (const auto& v : root["students"].toArray()) {
        const QString name = v.toString().trimmed();
        if (!name.isEmpty()) {
            m_students.append(name);
        }
    }

    m_buttons.clear();
    for (const auto& v : root["buttons"].toArray()) {
        const auto o = v.toObject();
        const QString name = o["name"].toString().trimmed();
        const QString action = o["action"].toString().trimmed();
        const QString target = o["target"].toString().trimmed();
        if (name.isEmpty() || action.isEmpty() || target.isEmpty()) {
            continue;
        }
        if (target == "classisland://open") {
            continue;
        }
        m_buttons.append({name,
                          o["icon"].toString().trimmed(),
                          action,
                          target,
                          o["isSystem"].toBool(false)});
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
    root["iconSize"] = iconSize;
    root["floatingOpacity"] = floatingOpacity;
    root["attendanceSummaryWidth"] = attendanceSummaryWidth;
    root["startCollapsed"] = startCollapsed;
    root["trayClickToOpen"] = trayClickToOpen;
    root["showAttendanceSummaryOnStart"] = showAttendanceSummaryOnStart;
    root["firstRunCompleted"] = firstRunCompleted;
    root["fixedSidebarWidth"] = kSidebarWidth;

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


void Config::resetToDefaults(bool preserveFirstRun) {
    const bool oldFirstRun = firstRunCompleted;
    applyDefaults(*this, m_buttons, m_students);
    if (preserveFirstRun) {
        firstRunCompleted = oldFirstRun;
    }
    save();
}
QVector<AppButton> Config::getButtons() const { return m_buttons; }

void Config::setButtons(const QVector<AppButton>& btns) {
    m_buttons = btns;
    save();
}

QStringList Config::getStudentList() const { return m_students; }

void Config::setStudentList(const QStringList& list) {
    m_students = list;
    save();
}

bool Config::importStudentsFromText(const QString& filePath, QString* errorMessage) {
    const QString suffix = QFileInfo(filePath).suffix().toLower();
    if (suffix == "xls" || suffix == "xlsx") {
        if (errorMessage) {
            *errorMessage = "当前版本不依赖第三方库，Excel 请先另存为 CSV 或 TXT 再导入。";
        }
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage) {
            *errorMessage = "名单文件读取失败。";
        }
        return false;
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
        for (const auto& part : parts) {
            const QString name = part.trimmed();
            if (!name.isEmpty()) {
                parsedStudents.append(name);
            }
        }
    }

    parsedStudents.removeDuplicates();
    if (parsedStudents.isEmpty()) {
        if (errorMessage) {
            *errorMessage = "文件中没有可用学生数据。";
        }
        return false;
    }

    m_students = parsedStudents;
    save();
    return true;
}
