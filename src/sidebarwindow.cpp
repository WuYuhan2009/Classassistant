#include "sidebarwindow.h"

#include "attendanceoverlay.h"
#include "configmanager.h"
#include "randomnamedialog.h"

#include <QDesktopServices>
#include <QFileInfo>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QRandomGenerator>
#include <QScreen>
#include <QUrl>
#include <QVBoxLayout>
#include <QStringList>

SidebarWindow::SidebarWindow(ConfigManager *config, AttendanceOverlay *overlay, QWidget *parent)
    : QWidget(parent), m_config(config), m_overlay(overlay), m_layout(new QVBoxLayout(this)) {
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground, true);

    auto *collapseBtn = new QPushButton("⇤");
    collapseBtn->setFixedHeight(30);
    connect(collapseBtn, &QPushButton::clicked, this, &SidebarWindow::collapseRequested);

    m_layout->setSpacing(10);
    m_layout->setContentsMargins(8, 8, 8, 8);
    m_layout->addWidget(collapseBtn);

    rebuildButtons();
}

void SidebarWindow::rebuildButtons() {
    while (m_layout->count() > 1) {
        auto *item = m_layout->takeAt(1);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    const auto &buttons = m_config->config().buttons;
    for (const auto &action : buttons) {
        auto *btn = new QPushButton(action.title);
        btn->setIcon(QIcon(action.iconPath));
        btn->setIconSize(QSize(m_config->config().theme.iconSize, m_config->config().theme.iconSize));
        btn->setMinimumWidth(m_config->config().theme.sidebarWidth - 16);
        connect(btn, &QPushButton::clicked, this, [this, action] {
            if (action.id == "default_settings") {
                emit settingsRequested();
                return;
            }
            if (action.id == "default_attendance") {
                openAttendanceEditor();
                return;
            }
            if (action.id == "default_random") {
                randomRollCall();
                return;
            }
            if (action.id == "default_classisland") {
                openClassIslandShortcut();
                return;
            }
            if (action.id == "default_whiteboard") {
                QString path = m_config->config().whiteboardPath;
                if (path.isEmpty()) {
                    QMessageBox::warning(this, "未配置", "请先在设置中配置希沃白板路径。");
                } else {
                    QProcess::startDetached(path, {});
                }
                return;
            }
            launchAction(action);
        });
        m_layout->addWidget(btn);
    }
    m_layout->addStretch(1);

    const QRect screenRect = QGuiApplication::primaryScreen()->availableGeometry();
    setGeometry(screenRect.right() - m_config->config().theme.sidebarWidth,
                screenRect.top() + 80,
                m_config->config().theme.sidebarWidth,
                screenRect.height() - 160);
}

void SidebarWindow::launchAction(const ButtonAction &action) {
    if (action.actionType == "url") {
        QDesktopServices::openUrl(QUrl(action.target));
        return;
    }
    if (action.actionType == "executable" || action.actionType == "file") {
        QProcess::startDetached(action.target, {});
    }
}

void SidebarWindow::openAttendanceEditor() {
    auto &att = m_config->config().attendance;
    if (att.allStudents.isEmpty()) {
        QMessageBox::information(this, "未导入名单", "请先在设置中导入班级名单。");
        return;
    }

    bool ok = false;
    const QString defaults = QStringList(att.absentStudents.toList()).join(",");
    const QString input = QInputDialog::getText(this, "班级考勤", "请输入未出勤学生（逗号分隔）", QLineEdit::Normal, defaults, &ok);
    if (!ok) return;

    att.absentStudents = input.split(",", Qt::SkipEmptyParts).toVector();
    for (QString &name : att.absentStudents) {
        name = name.trimmed();
    }
    m_config->save();
    m_overlay->refresh();
}

void SidebarWindow::randomRollCall() {
    const auto &students = m_config->config().attendance.allStudents;
    if (students.isEmpty()) {
        QMessageBox::information(this, "提示", "请先导入班级名单。");
        return;
    }
    const int idx = QRandomGenerator::global()->bounded(students.size());
    RandomNameDialog dialog(students[idx], this);
    dialog.exec();
}

void SidebarWindow::openClassIslandShortcut() {
#ifdef Q_OS_WIN
    if (!QDesktopServices::openUrl(QUrl("classisland://switch?next=1"))) {
        QMessageBox::warning(this, "调用失败", "未检测到 ClassIsland URL Scheme。");
    }
#else
    if (!QProcess::startDetached("classisland", QStringList() << "--next")) {
        QMessageBox::warning(this, "调用失败", "Linux 下未找到 classisland 命令，请在设置系统路径后重试。");
    }
#endif
}
