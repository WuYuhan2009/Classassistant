#include "Sidebar.h"

#include "../Utils.h"
#include "FluentTheme.h"

#include <QDesktopServices>
#include <QIcon>
#include <QMessageBox>
#include <QProcess>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QUrl>

namespace {
constexpr int kSidebarMinWidth = 84;
}

Sidebar::Sidebar(QWidget* parent) : QWidget(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    FluentTheme::applyWinUIWindowShadow(this);

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

QList<QWidget*> Sidebar::managedToolWindows() const {
    return {
        m_attendanceSelector,
        m_randomCall,
        m_classTimer,
        m_classNote,
        m_groupSplit,
        m_scoreBoard,
        m_aiAssistant,
        m_settings,
    };
}

void Sidebar::showManagedWindow(QWidget* window) {
    if (!window) {
        return;
    }
    window->setWindowOpacity(1.0);
    window->show();
    window->raise();
    window->activateWindow();
}

QPushButton* Sidebar::createIconButton(const QString& text,
                                       const QString& iconPath,
                                       const QString& tooltip,
                                       const QString& fallbackEmoji) {
    auto* btn = new QPushButton(text);
    const bool compact = Config::instance().compactMode;
    const int width = qMax(kSidebarMinWidth, Config::instance().sidebarWidth);
    const int side = compact ? (width - 24) : (width - 12);
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

    btn->setStyleSheet(FluentTheme::sidebarButtonStyle());
    return btn;
}

void Sidebar::rebuildUI() {
    while (QLayoutItem* item = m_layout->takeAt(0)) {
        delete item->widget();
        delete item;
    }

    m_layout->setSpacing(Config::instance().compactMode ? 5 : 10);
    setFixedWidth(qMax(kSidebarMinWidth, Config::instance().sidebarWidth));
    setStyleSheet(FluentTheme::sidebarPanelStyle());

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
    showManagedWindow(m_settings);
}

void Sidebar::triggerTool(const QString& target) {
    handleAction("func", target);
}

void Sidebar::hideAllToolWindowsAnimated() {
    if (!Config::instance().collapseHidesToolWindows) {
        return;
    }

    const int duration = Config::instance().animationDurationMs;
    const auto hideWidget = [duration](QWidget* window) {
        if (!window || !window->isVisible()) {
            return;
        }

        auto* anim = new QPropertyAnimation(window, "windowOpacity", window);
        anim->setDuration(duration);
        anim->setStartValue(window->windowOpacity());
        anim->setEndValue(0.0);
        QObject::connect(anim, &QPropertyAnimation::finished, window, [window]() {
            window->hide();
            window->setWindowOpacity(1.0);
        });
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    };

    for (QWidget* window : managedToolWindows()) {
        hideWidget(window);
    }
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

void Sidebar::launchExecutableTarget(const QString& target) {
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
}

void Sidebar::launchUrlTarget(const QString& target) {
    if (!Config::instance().allowExternalLinks) {
        QMessageBox::information(this, "已禁用", "当前处于离线优先模式，URL 打开已禁用。可在设置-安全与离线中启用。");
        return;
    }
    QDesktopServices::openUrl(QUrl(target));
}

void Sidebar::handleFunctionAction(const QString& target) {
    if (target == "ATTENDANCE") {
        m_attendanceSummary->show();
        m_attendanceSummary->raise();
        showManagedWindow(m_attendanceSelector);
    } else if (target == "RANDOM_CALL") {
        m_randomCall->setWindowOpacity(1.0);
        m_randomCall->startAnim();
    } else if (target == "CLASS_TIMER") {
        m_classTimer->setWindowOpacity(1.0);
        m_classTimer->openTimer();
    } else if (target == "CLASS_NOTE") {
        m_classNote->setWindowOpacity(1.0);
        m_classNote->openNote();
    } else if (target == "GROUP_SPLIT") {
        m_groupSplit->setWindowOpacity(1.0);
        m_groupSplit->openSplitter();
    } else if (target == "SCORE_BOARD") {
        m_scoreBoard->setWindowOpacity(1.0);
        m_scoreBoard->openBoard();
    } else if (target == "AI_ASSISTANT") {
        m_aiAssistant->setWindowOpacity(1.0);
        m_aiAssistant->openAssistant();
    } else if (target == "SETTINGS") {
        openSettings();
    }
}

void Sidebar::handleAction(const QString& action, const QString& target) {
    if (action == "exe") {
        launchExecutableTarget(target);
        return;
    }

    if (action == "url") {
        launchUrlTarget(target);
        return;
    }

    if (action == "func") {
        handleFunctionAction(target);
    }
}
