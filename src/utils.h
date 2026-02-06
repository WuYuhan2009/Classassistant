#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QStringList>

class Utils
{
public:
    static bool fileExists(const QString &path);
    static bool downloadResource(const QString &url, const QString &savePath);
    static QStringList parseStudentFile(const QString &filePath);
    static QString getAppDataPath();
    static bool isClassIslandInstalled();
    static QString getClassIslandVersion();
private:
    Utils() = delete;
};

#endif
