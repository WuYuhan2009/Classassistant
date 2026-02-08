#include "Utils.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTextStream>
#include <QTimer>

namespace {
constexpr int kSidebarWidth = 84;

const QMap<QString, QString>& remoteIconMap() {
    static const QMap<QString, QString> kMap = {
        {"icon_seewo.png", "https://upload.cc/i1/2026/02/08/Y6wmA8.png"},
        {"icon_attendance.png", "https://upload.cc/i1/2026/02/08/HNo35p.png"},
        {"icon_random.png", "https://upload.cc/i1/2026/02/08/Dt8WIg.png"},
        {"icon_ai.png", "https://upload.cc/i1/2026/02/08/GeojsQ.png"},
        {"icon_settings.png", "https://upload.cc/i1/2026/02/08/vCRlDF.png"},
        {"icon_collapse.png", "https://upload.cc/i1/2026/02/08/BTjyOR.png"},
        {"icon_expand.png", "https://upload.cc/i1/2026/02/08/N59bqp.png"},
    };
    return kMap;
}

QVector<AppButton> defaultButtons() {
    return {
        {"希沃白板", "icon_seewo.png", "exe", "SEEWO", true},
        {"班级考勤", "icon_attendance.png", "func", "ATTENDANCE", true},
        {"随机点名", "icon_random.png", "func", "RANDOM_CALL", true},
        {"AI 助手", "icon_ai.png", "url", "https://www.doubao.com/chat/", true},
    };
}

QStringList defaultStudents() {
    return QStringList{QStringLiteral("张三"), QStringLiteral("李四"), QStringLiteral("王五"), QStringLiteral("赵六"), QStringLiteral("示例学生")};
}

QString iconCacheDirPath() {
    const QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return dataPath + "/icons";
}

QString httpCacheFilePath(const QString& iconRef) {
    const QByteArray md5 = QCryptographicHash::hash(iconRef.toUtf8(), QCryptographicHash::Md5).toHex();
    return iconCacheDirPath() + "/" + QString::fromLatin1(md5) + ".png";
}

QString namedCacheFilePath(const QString& fileName) {
    return iconCacheDirPath() + "/" + fileName;
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

bool downloadToFile(QNetworkAccessManager& manager, const QUrl& url, const QString& outPath) {
    QNetworkRequest request(url);
#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
#endif
    QNetworkReply* reply = manager.get(request);

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timeout.start(4000);
    loop.exec();

    if (timeout.isActive()) {
        timeout.stop();
    } else {
        reply->abort();
    }

    const bool ok = reply->error() == QNetworkReply::NoError;
    const QByteArray data = reply->readAll();
    reply->deleteLater();

    if (!ok || data.isEmpty()) {
        return false;
    }

    QFile out(outPath);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    out.write(data);
    return true;
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

QString Config::resolveIconPath(const QString& iconRef) const {
    if (iconRef.isEmpty()) {
        return {};
    }

    if (iconRef.startsWith(":/")) {
        return iconRef;
    }

    if (iconRef.startsWith("http://") || iconRef.startsWith("https://")) {
        const QString cachedHttpPath = httpCacheFilePath(iconRef);
        if (QFile::exists(cachedHttpPath)) {
            return cachedHttpPath;
        }
        return {};
    }

    if (QFileInfo(iconRef).isAbsolute() && QFile::exists(iconRef)) {
        return iconRef;
    }

    const QString namedCachedPath = namedCacheFilePath(iconRef);
    if (QFile::exists(namedCachedPath)) {
        return namedCachedPath;
    }

    const QString appDirAsset = QCoreApplication::applicationDirPath() + "/assets/" + iconRef;
    if (QFile::exists(appDirAsset)) {
        return appDirAsset;
    }

    const QString currentDirAsset = QDir::currentPath() + "/assets/" + iconRef;
    if (QFile::exists(currentDirAsset)) {
        return currentDirAsset;
    }

    return QString(":/assets/%1").arg(iconRef);
}

void Config::ensureRemoteIconCache() {
    if (m_remoteIconCacheAttempted) {
        return;
    }
    m_remoteIconCacheAttempted = true;

    QDir().mkpath(iconCacheDirPath());
    QNetworkAccessManager manager;
    const auto iconMap = remoteIconMap();
    for (auto it = iconMap.constBegin(); it != iconMap.constEnd(); ++it) {
        const QString localPath = namedCacheFilePath(it.key());
        if (QFile::exists(localPath)) {
            continue;
        }
        downloadToFile(manager, QUrl(it.value()), localPath);
    }
}
