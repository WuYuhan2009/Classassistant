#include "Sidebar.h"

#include "../Utils.h"

#include <QDesktopServices>
#include <QFile>
#include <QIcon>
#include <QProcess>
#include <QPushButton>
#include <QUrl>

namespace {
constexpr int kSidebarWidth = 84;
}

Sidebar::Sidebar(QWidget* parent) : QWidget(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    m_attendanceSummary = new AttendanceSummaryWidget();
    if (Config::instance().showAttendanceSummaryOnStart) {
        m_attendanceSummary->show();
    }
    m_attendanceSelector = new AttendanceSelectDialog();
    m_randomCall = new RandomCallDialog();
    m_settings = new SettingsDialog();

    connect(m_settings, &SettingsDialog::configChanged, this, &Sidebar::reloadConfig);
    connect(m_attendanceSelector, &AttendanceSelectDialog::saved, m_attendanceSummary, &AttendanceSummaryWidget::applyAbsentees);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(6, 12, 6, 12);
    m_layout->setSpacing(8);

    rebuildUI();
}

QPushButton* Sidebar::createIconButton(const QString& text,
                                       const QString& iconPath,
                                       const QString& tooltip,
                                       const QString& fallbackEmoji) {
    auto* btn = new QPushButton(text);
    btn->setFixedSize(kSidebarWidth - 12, kSidebarWidth - 12);
    btn->setToolTip(tooltip);

    const QIcon icon(iconPath);
    if (!icon.isNull()) {
        btn->setIcon(icon);
        btn->setIconSize(QSize(Config::instance().iconSize, Config::instance().iconSize));
        btn->setText("");
    } else if (!fallbackEmoji.isEmpty()) {
        btn->setText(fallbackEmoji);
    }

    btn->setStyleSheet("background: rgba(255,255,255,0.96); border: 1px solid #cfd5dd; border-radius: 12px; font-size: 22px;");
    return btn;
}

void Sidebar::rebuildUI() {
    while (QLayoutItem* item = m_layout->takeAt(0)) {
        if (item->widget()) delete item->widget();
        delete item;
    }

    setFixedWidth(kSidebarWidth);
    setStyleSheet("QWidget { background-color: rgba(244, 248, 252, 0.95); border-top-left-radius: 14px; border-bottom-left-radius: 14px; }");

    m_layout->addStretch();
    const auto buttons = Config::instance().getButtons();
    for (const auto& b : buttons) {
        auto* btn = createIconButton(b.name.left(2), b.iconPath, b.name, "🔘");
        connect(btn, &QPushButton::clicked, [this, b]() { handleAction(b.action, b.target); });
        m_layout->addWidget(btn, 0, Qt::AlignHCenter);
    }

    auto* settingsBtn = createIconButton("设", ":/assets/icon_settings.png", "设置", "⚙️");
    connect(settingsBtn, &QPushButton::clicked, this, &Sidebar::openSettings);
    m_layout->addWidget(settingsBtn, 0, Qt::AlignHCenter);

    auto* collapseBtn = createIconButton("收", ":/assets/icon_collapse.png", "收起", "⏷");
    connect(collapseBtn, &QPushButton::clicked, this, &Sidebar::requestHide);
    m_layout->addWidget(collapseBtn, 0, Qt::AlignHCenter);
    m_layout->addStretch();
}

void Sidebar::openSettings() {
    m_settings->show();
    m_settings->raise();
    m_settings->activateWindow();
}

void Sidebar::reloadConfig() {
    Config::instance().load();
    rebuildUI();
    m_attendanceSummary->resetDaily();
    if (Config::instance().showAttendanceSummaryOnStart) {
        m_attendanceSummary->show();
    } else {
        m_attendanceSummary->hide();
    }
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
            m_attendanceSummary->show();
            m_attendanceSummary->raise();
            m_attendanceSelector->show();
            m_attendanceSelector->raise();
        } else if (target == "RANDOM_CALL") {
            m_randomCall->startAnim();
        } else if (target == "SETTINGS") {
            openSettings();
        }
    }
}
