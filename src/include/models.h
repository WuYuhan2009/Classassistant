#pragma once

#include <QString>
#include <QVector>

struct AppTheme {
    bool darkMode = true;
    int sidebarWidth = 84;
    int iconSize = 30;
};

struct ButtonAction {
    QString id;
    QString title;
    QString iconPath;
    QString target;
    QString actionType; // executable, file, url, internal
    bool removable = true;
};

struct AttendanceRecord {
    QVector<QString> allStudents;
    QVector<QString> absentStudents;
    QString lastDate;
};

struct AppConfig {
    bool firstRunCompleted = false;
    AppTheme theme;
    QString whiteboardPath;
    QVector<ButtonAction> buttons;
    AttendanceRecord attendance;
};
