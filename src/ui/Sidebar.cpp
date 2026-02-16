#include "Sidebar.h"

#include "../Utils.h"

#include <QDesktopServices>
#include <QIcon>
#include <QMessageBox>
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
    m_classTimer = new ClassTimerDialog();
    m_classNote = new ClassNoteDialog();
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
    const bool compact = Config::instance().compactMode;
    const int side = compact ? (kSidebarWidth - 24) : (kSidebarWidth - 12);
    btn->setFixedSize(side, side);
    btn->setToolTip(tooltip);

    const QIcon icon(Config::instance().resolveIconPath(iconPath));
    if (!icon.isNull()) {
        btn->setIcon(icon);
        const int iconSide = qMin(compact ? 30 : 40, qMax(20, Config::instance().iconSize));
        btn->setIconSize(QSize(iconSide, iconSide));
        btn->setText("");
    } else if (!fallbackEmoji.isEmpty()) {
        btn->setText(fallbackEmoji);
    }

    btn->setStyleSheet("background: rgba(255,255,255,0.96); border: 1px solid #d6deea; border-radius: 12px; font-size: 20px; padding: 4px;");
    return btn;
}

void Sidebar::rebuildUI() {
    while (QLayoutItem* item = m_layout->takeAt(0)) {
        delete item->widget();
        delete item;
    }

    m_layout->setSpacing(Config::instance().compactMode ? 4 : 8);
    setFixedWidth(kSidebarWidth);
    setStyleSheet("QWidget { background-color: rgba(250, 252, 255, 0.97); border-top-left-radius: 16px; border-bottom-left-radius: 16px; }");

    m_layout->addStretch();
    const auto buttons = Config::instance().getButtons();
    for (const auto& b : buttons) {
        auto* btn = createIconButton(b.name.left(2), b.iconPath, b.name, "🔘");
        connect(btn, &QPushButton::clicked, [this, b]() { handleAction(b.action, b.target); });
        m_layout->addWidget(btn, 0, Qt::AlignHCenter);
    }

    auto* settingsBtn = createIconButton("设", "icon_settings.png", "设置", "⚙️");
    connect(settingsBtn, &QPushButton::clicked, this, &Sidebar::openSettings);
    m_layout->addWidget(settingsBtn, 0, Qt::AlignHCenter);

    auto* collapseBtn = createIconButton("收", "icon_collapse.png", "收起", "⏷");
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
        if (path.trimmed().isEmpty()) {
            QMessageBox::warning(this, "启动失败", "程序路径为空，请在设置中配置后重试。");
            return;
        }
        if (!QProcess::startDetached(path, {})) {
            if (!QDesktopServices::openUrl(QUrl::fromLocalFile(path))) {
                QMessageBox::warning(this, "启动失败", QString("无法打开目标：%1").arg(path));
            }
        }
    } else if (action == "url") {
        if (!Config::instance().allowExternalLinks) {
            QMessageBox::information(this, "已禁用", "当前处于离线优先模式，URL 打开已禁用。可在设置-安全与离线中启用。");
            return;
        }
        QDesktopServices::openUrl(QUrl(target));
    } else if (action == "func") {
        if (target == "ATTENDANCE") {
            m_attendanceSummary->show();
            m_attendanceSummary->raise();
            m_attendanceSelector->show();
            m_attendanceSelector->raise();
        } else if (target == "RANDOM_CALL") {
            m_randomCall->startAnim();
        } else if (target == "CLASS_TIMER") {
            m_classTimer->openTimer();
        } else if (target == "CLASS_NOTE") {
            m_classNote->openNote();
        } else if (target == "SETTINGS") {
            openSettings();
        }
    }
}
