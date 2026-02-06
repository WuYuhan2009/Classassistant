#include "attendanceoverlay.h"

#include "configmanager.h"

#include <QGuiApplication>
#include <QLabel>
#include <QScreen>
#include <QVBoxLayout>
#include <QStringList>

AttendanceOverlay::AttendanceOverlay(ConfigManager *config, QWidget *parent)
    : QWidget(parent), m_config(config), m_summary(new QLabel(this)) {
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint);
    setAttribute(Qt::WA_TransparentForMouseEvents, false);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_summary);
    layout->setContentsMargins(10, 10, 10, 10);

    refresh();

    QRect rect = QGuiApplication::primaryScreen()->availableGeometry();
    setGeometry(rect.right() - 360, rect.bottom() - 160, 340, 140);
}

void AttendanceOverlay::refresh() {
    const auto &att = m_config->config().attendance;
    const int all = att.allStudents.size();
    const int absent = att.absentStudents.size();
    const int actual = qMax(0, all - absent);

    m_summary->setText(QString("应到：%1\n实到：%2\n请假：%3")
                           .arg(all)
                           .arg(actual)
                           .arg(QStringList(att.absentStudents.toList()).join("、")));
}
