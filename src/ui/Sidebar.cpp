#include "Sidebar.h"

#include "../Utils.h"

#include <QCloseEvent>
#include <QDesktopServices>
#include <QFile>
#include <QIcon>
#include <QProcess>
#include <QPushButton>
#include <QUrl>

Sidebar::Sidebar(QWidget* parent) : QWidget(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    m_attendance = new AttendanceWidget();
    m_randomCall = new RandomCallDialog();
    m_settings = new SettingsDialog();

    connect(m_settings, &SettingsDialog::configChanged, this, &Sidebar::reloadConfig);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(5, 20, 5, 20);
    m_layout->setSpacing(10);

    rebuildUI();
}

void Sidebar::rebuildUI() {
    while (QLayoutItem* item = m_layout->takeAt(0)) {
        if (item->widget()) delete item->widget();
        delete item;
    }

    setFixedWidth(Config::instance().sidebarWidth);
    setStyleSheet("QWidget { background-color: rgba(240, 240, 240, 0.95); border-top-left-radius: 10px; border-bottom-left-radius: 10px; }");

    const auto buttons = Config::instance().getButtons();
    for (const auto& btnData : buttons) {
        if (btnData.target == "SETTINGS" && btnData.action == "func") {
            continue;
        }

        auto* btn = new QPushButton(btnData.name.left(2));
        btn->setFixedSize(Config::instance().sidebarWidth - 10, Config::instance().sidebarWidth - 10);
        btn->setToolTip(btnData.name);

        const QIcon icon(btnData.iconPath);
        if (!icon.isNull()) {
            btn->setIcon(icon);
            btn->setIconSize(QSize(Config::instance().iconSize, Config::instance().iconSize));
            btn->setStyleSheet("background-color: white; border-radius: 10px; border: 1px solid #ddd;");
            btn->setText("");
        } else if (QFile::exists(btnData.iconPath)) {
            btn->setStyleSheet(QString("border-image: url(%1); border: none;").arg(btnData.iconPath));
            btn->setText("");
        } else {
            btn->setStyleSheet("background-color: white; border-radius: 10px; border: 1px solid #ccc; font-weight: bold; color: #333;");
        }

        connect(btn, &QPushButton::clicked, [this, btnData]() { handleAction(btnData.action, btnData.target); });
        m_layout->addWidget(btn);
    }

    m_layout->addStretch();

    auto* settingsBtn = new QPushButton("设置");
    settingsBtn->setFixedSize(Config::instance().sidebarWidth - 10, 42);
    settingsBtn->setStyleSheet("background-color: #f7f7f7; border-radius: 6px; border: 1px solid #ddd;");
    connect(settingsBtn, &QPushButton::clicked, this, &Sidebar::openSettings);
    m_layout->addWidget(settingsBtn);

    auto* collapseBtn = new QPushButton("收起");
    collapseBtn->setFixedSize(Config::instance().sidebarWidth - 10, 42);
    collapseBtn->setStyleSheet("background-color: #ddd; border-radius: 6px;");
    connect(collapseBtn, &QPushButton::clicked, this, &Sidebar::requestHide);
    m_layout->addWidget(collapseBtn);
}

void Sidebar::openSettings() {
    m_settings->show();
    m_settings->raise();
    m_settings->activateWindow();
}

void Sidebar::reloadConfig() {
    Config::instance().load();
    m_attendance->resetDaily();
    rebuildUI();
}

void Sidebar::closeEvent(QCloseEvent* event) {
    hide();
    event->ignore();
}

void Sidebar::handleAction(const QString& action, const QString& target) {
    if (action == "exe") {
        const QString path = (target == "SEEWO") ? Config::instance().seewoPath : target;
        if (!QProcess::startDetached(path, {})) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(path));
        }
    } else if (action == "url") {
        QDesktopServices::openUrl(QUrl(target));
    } else if (action == "func") {
        if (target == "ATTENDANCE") {
            if (m_attendance->isVisible()) {
                m_attendance->hide();
            } else {
                m_attendance->show();
                m_attendance->raise();
            }
        } else if (target == "RANDOM_CALL") {
            m_randomCall->startAnim();
        } else if (target == "SETTINGS") {
            openSettings();
        }
    }
}
