#include "utils.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QSettings>

bool Utils::fileExists(const QString &path) {
    QFileInfo fileInfo(path);
    return fileInfo.exists() && fileInfo.isFile();
}

bool Utils::downloadResource(const QString &url, const QString &savePath) {
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(QNetworkRequest(QUrl(url)));
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    if (reply->error() != QNetworkReply::NoError) { reply->deleteLater(); return false; }
    QFile file(savePath);
    if (!file.open(QIODevice::WriteOnly)) { reply->deleteLater(); return false; }
    file.write(reply->readAll());
    reply->deleteLater();
    return true;
}

QStringList Utils::parseStudentFile(const QString &filePath) {
    QStringList students;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return students;
    QTextStream in(&file);
    in.setCodec("UTF-8");
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (!line.isEmpty()) students << line;
    }
    return students;
}

QString Utils::getAppDataPath() { return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation); }

bool Utils::isClassIslandInstalled() {
#ifdef Q_OS_WIN
    QSettings registry("HKEY_CURRENT_USER\\Software\\Classes\\classisland", QSettings::NativeFormat);
    return registry.contains("URL Protocol");
#else
    return QDir(QDir::homePath() + "/.config/ClassIsland").exists();
#endif
}

QString Utils::getClassIslandVersion() {
#ifdef Q_OS_WIN
    QSettings registry("HKEY_CURRENT_USER\\Software\\ClassIsland", QSettings::NativeFormat);
    return registry.value("Version", "未知").toString();
#else
    return "未知";
#endif
}
