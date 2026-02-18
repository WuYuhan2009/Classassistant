#include "Sidebar.h"

#include "../Utils.h"

#include <QDesktopServices>
#include <QIcon>
#include <QMessageBox>
#include <QProcess>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QUrl>
#include <functional>

namespace {
constexpr int kSidebarMinWidth = 84;
constexpr int kSidebarMarginCompact = 16;
constexpr int kSidebarMarginDefault = 24;
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
    m_groupSplit = new GroupSplitDialog();
    m_scoreBoard = new ScoreBoardDialog();
    m_aiAssistant = new AIAssistantDialog();
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
    const int width = qMax(kSidebarMinWidth, Config::instance().sidebarWidth);
    const int side = compact ? (width - kSidebarMarginCompact) : (width - kSidebarMarginDefault);
    btn->setFixedSize(side, side);
    btn->setToolTip(tooltip);

    const QIcon icon(Config::instance().resolveIconPath(iconPath));
    if (!icon.isNull()) {
        btn->setIcon(icon);
        const int iconSide = qMin(compact ? 28 : 40, qMax(20, Config::instance().iconSize));
        btn->setIconSize(QSize(iconSide, iconSide));
        btn->setText("");
    } else if (!fallbackEmoji.isEmpty()) {
        btn->setText(fallbackEmoji);
    }

    btn->setStyleSheet("QPushButton{background:#fffbfe;border:1px solid #79747e;border-radius:22px;font-size:20px;padding:6px;color:#1d1b20;}"
                       "QPushButton:hover{background:#f5f0fa;border-color:#625b71;}"
                       "QPushButton:pressed{background:#ece6f0;}"
                       "QPushButton:focus{border:2px solid #6750a4;}");
    return btn;
}

void Sidebar::rebuildUI() {
    while (QLayoutItem* item = m_layout->takeAt(0)) {
        delete item->widget();
        delete item;
    }

    m_layout->setSpacing(Config::instance().compactMode ? 6 : 10);
    setFixedWidth(qMax(kSidebarMinWidth, Config::instance().sidebarWidth));
    setStyleSheet("QWidget { background-color: rgba(254, 247, 255, 0.98); border:1px solid #e7e0ec; border-top-left-radius: 28px; border-bottom-left-radius: 28px; }");

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

void Sidebar::triggerTool(const QString& target) {
    handleAction("func", target);
}

void Sidebar::hideAllToolWindowsAnimated() {
    if (!Config::instance().collapseHidesToolWindows) {
        return;
    }

    const int duration = Config::instance().animationDurationMs;
    const auto hideWidget = [this, duration](QWidget* w) {
        if (!w || !w->isVisible()) {
            return;
        }
        auto* anim = new QPropertyAnimation(w, "windowOpacity", w);
        anim->setDuration(duration);
        anim->setStartValue(w->windowOpacity());
        anim->setEndValue(0.0);
        QObject::connect(anim, &QPropertyAnimation::finished, w, &QWidget::hide);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    };

    hideWidget(m_attendanceSelector);
    hideWidget(m_randomCall);
    hideWidget(m_classTimer);
    hideWidget(m_classNote);
    hideWidget(m_groupSplit);
    hideWidget(m_scoreBoard);
    hideWidget(m_aiAssistant);
    hideWidget(m_settings);
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
    hideAllToolWindowsAnimated();
    hide();
    event->ignore();
}

void Sidebar::handleAction(const QString& action, const QString& target) {
    const auto showToolDialog = [](QWidget* dialog, const std::function<void()>& opener) {
        if (!dialog) {
            return;
        }
        dialog->setWindowOpacity(1.0);
        opener();
    };

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
            showToolDialog(m_attendanceSelector, [this]() {
                m_attendanceSelector->show();
                m_attendanceSelector->raise();
            });
        } else if (target == "RANDOM_CALL") {
            showToolDialog(m_randomCall, [this]() { m_randomCall->startAnim(); });
        } else if (target == "CLASS_TIMER") {
            showToolDialog(m_classTimer, [this]() { m_classTimer->openTimer(); });
        } else if (target == "CLASS_NOTE") {
            showToolDialog(m_classNote, [this]() { m_classNote->openNote(); });
        } else if (target == "GROUP_SPLIT") {
            showToolDialog(m_groupSplit, [this]() { m_groupSplit->openSplitter(); });
        } else if (target == "SCORE_BOARD") {
            showToolDialog(m_scoreBoard, [this]() { m_scoreBoard->openBoard(); });
        } else if (target == "AI_ASSISTANT") {
            showToolDialog(m_aiAssistant, [this]() { m_aiAssistant->openAssistant(); });
        } else if (target == "SETTINGS") {
            showToolDialog(m_settings, [this]() { openSettings(); });
        }
    }
}
