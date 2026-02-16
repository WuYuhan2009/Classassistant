#include "Utils.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

namespace {
constexpr int kSidebarWidth = 84;

QVector<AppButton> defaultButtons() {
    return {
        {"希沃白板", "icon_seewo.png", "exe", "SEEWO", true},
        {"班级考勤", "icon_attendance.png", "func", "ATTENDANCE", true},
        {"随机点名", "icon_random.png", "func", "RANDOM_CALL", true},
        {"课堂计时", "icon_settings.png", "func", "CLASS_TIMER", true},
        {"课堂便签", "icon_settings.png", "func", "CLASS_NOTE", true},
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
    config.randomNoRepeat = true;
    config.allowExternalLinks = false;
    config.compactMode = false;
    config.randomHistorySize = 5;
    config.firstRunCompleted = false;
    config.classNote = "";
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
    randomNoRepeat = root["randomNoRepeat"].toBool(true);
    allowExternalLinks = root["allowExternalLinks"].toBool(false);
    compactMode = root["compactMode"].toBool(false);
    randomHistorySize = qBound(3, root["randomHistorySize"].toInt(5), 10);
    firstRunCompleted = root["firstRunCompleted"].toBool(false);
    classNote = root["classNote"].toString();

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
        QString iconRef = o["icon"].toString().trimmed();
        if (iconRef.startsWith(":/assets/")) {
            iconRef = iconRef.mid(QString(":/assets/").size());
        }
        m_buttons.append({name,
                          iconRef,
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
    root["randomNoRepeat"] = randomNoRepeat;
    root["allowExternalLinks"] = allowExternalLinks;
    root["compactMode"] = compactMode;
    root["randomHistorySize"] = randomHistorySize;
    root["firstRunCompleted"] = firstRunCompleted;
    root["classNote"] = classNote;
    root["fixedSidebarWidth"] = kSidebarWidth;

    QJsonArray stuArr;
    for (auto it = m_students.cbegin(); it != m_students.cend(); ++it) {
        stuArr.append(*it);
    }
    root["students"] = stuArr;

    QJsonArray btnArr;
    for (auto it = m_buttons.cbegin(); it != m_buttons.cend(); ++it) {
        const AppButton& b = *it;
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

QString Config::resolveIconPath(const QString& iconRef) const {
    if (iconRef.isEmpty()) {
        return {};
    }

    if (iconRef.startsWith(":/")) {
        return iconRef;
    }

    if (QFileInfo(iconRef).isAbsolute() && QFile::exists(iconRef)) {
        return iconRef;
    }

    const QString appDirIcons = QCoreApplication::applicationDirPath() + "/assets/icons/" + iconRef;
    if (QFile::exists(appDirIcons)) {
        return appDirIcons;
    }

    const QString appDirAsset = QCoreApplication::applicationDirPath() + "/assets/" + iconRef;
    if (QFile::exists(appDirAsset)) {
        return appDirAsset;
    }

    const QString currentDirIcons = QDir::currentPath() + "/assets/icons/" + iconRef;
    if (QFile::exists(currentDirIcons)) {
        return currentDirIcons;
    }

    const QString currentDirAsset = QDir::currentPath() + "/assets/" + iconRef;
    if (QFile::exists(currentDirAsset)) {
        return currentDirAsset;
    }

    return {};
}
