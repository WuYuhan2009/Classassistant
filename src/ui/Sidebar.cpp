#include "Sidebar.h"

#include "../Utils.h"

#include <QDesktopServices>
#include <QIcon>
#include <QMessageBox>
#include <QProcess>
#include <QDateTime>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QUrl>
#include <functional>

namespace {
constexpr int kSidebarMinWidth = 84;
constexpr int kSidebarMarginCompact = 16;
constexpr int kSidebarMarginDefault = 24;

QString navButtonStyle() {
    return "QPushButton{background:#fffbfe;border:1px solid #79747e;border-radius:22px;font-size:18px;padding:6px;color:#1d1b20;}"
           "QPushButton:hover{background:#ece6f0;border-color:#625b71;}"
           "QPushButton:pressed{background:#ded8e8;}"
           "QPushButton:checked{background:#e8def8;border:1px solid #e8def8;color:#1d192b;}"
           "QPushButton:focus{border:2px solid #6750a4;}";
}
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

    auto* clockTimer = new QTimer(this);
    connect(clockTimer, &QTimer::timeout, this, &Sidebar::updateClockText);
    clockTimer->start(1000);

    rebuildUI();
    updateClockText();
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

    btn->setCheckable(true);
    btn->setStyleSheet(navButtonStyle());
    return btn;
}

void Sidebar::rebuildUI() {
    while (QLayoutItem* item = m_layout->takeAt(0)) {
        delete item->widget();
        delete item;
    }
    m_toolButtons.clear();

    m_layout->setSpacing(Config::instance().compactMode ? 6 : 10);
    setFixedWidth(qMax(kSidebarMinWidth, Config::instance().sidebarWidth));
    setStyleSheet("QWidget { background-color: rgba(254, 247, 255, 0.98); border:1px solid #e7e0ec; border-top-left-radius: 28px; border-bottom-left-radius: 28px; }");

    auto* title = new QLabel("班级小助手");
    title->setProperty("md3Role", "headline");
    title->setAlignment(Qt::AlignHCenter);

    m_clockLabel = new QLabel;
    m_clockLabel->setProperty("md3Role", "bodyMuted");
    m_clockLabel->setAlignment(Qt::AlignHCenter);

    auto* heroCard = new QWidget;
    heroCard->setStyleSheet("background:#f3edf7;border:1px solid #e7e0ec;border-radius:18px;");
    auto* heroLayout = new QVBoxLayout(heroCard);
    heroLayout->setContentsMargins(10, 10, 10, 10);
    heroLayout->setSpacing(4);
    heroLayout->addWidget(title);
    heroLayout->addWidget(m_clockLabel);

    m_layout->addWidget(heroCard);
    m_layout->addSpacing(2);
    m_layout->addStretch();

    const auto buttons = Config::instance().getButtons();
    for (const auto& b : buttons) {
        auto* btn = createIconButton(b.name.left(2), b.iconPath, b.name, "🔘");
        connect(btn, &QPushButton::clicked, [this, b]() {
            setActiveTool(b.target);
            handleAction(b.action, b.target);
        });
        m_layout->addWidget(btn, 0, Qt::AlignHCenter);
        m_toolButtons.insert(b.target, btn);
    }

    auto* settingsBtn = createIconButton("设", "icon_settings.png", "设置", "⚙️");
    connect(settingsBtn, &QPushButton::clicked, [this]() {
        setActiveTool("SETTINGS");
        openSettings();
    });
    m_layout->addWidget(settingsBtn, 0, Qt::AlignHCenter);
    m_toolButtons.insert("SETTINGS", settingsBtn);

    auto* collapseBtn = createIconButton("收", "icon_collapse.png", "收起", "⏷");
    collapseBtn->setCheckable(false);
    collapseBtn->setProperty("md3Role", "tonal");
    connect(collapseBtn, &QPushButton::clicked, this, &Sidebar::requestHide);
    m_layout->addWidget(collapseBtn, 0, Qt::AlignHCenter);
    m_layout->addStretch();
}


void Sidebar::updateClockText() {
    if (!m_clockLabel) {
        return;
    }
    const QString stamp = QDateTime::currentDateTime().toString("MM-dd ddd HH:mm:ss");
    m_clockLabel->setText(QString("教学进行中 · %1").arg(stamp));
}

void Sidebar::setActiveTool(const QString& target) {
    for (auto it = m_toolButtons.begin(); it != m_toolButtons.end(); ++it) {
        if (it.value()) {
            it.value()->setChecked(it.key() == target);
        }
    }
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
            setActiveTool(target);
            m_attendanceSummary->show();
            m_attendanceSummary->raise();
            showToolDialog(m_attendanceSelector, [this]() {
                m_attendanceSelector->show();
                m_attendanceSelector->raise();
            });
        } else if (target == "RANDOM_CALL") {
            setActiveTool(target);
            showToolDialog(m_randomCall, [this]() { m_randomCall->startAnim(); });
        } else if (target == "CLASS_TIMER") {
            setActiveTool(target);
            showToolDialog(m_classTimer, [this]() { m_classTimer->openTimer(); });
        } else if (target == "CLASS_NOTE") {
            setActiveTool(target);
            showToolDialog(m_classNote, [this]() { m_classNote->openNote(); });
        } else if (target == "GROUP_SPLIT") {
            setActiveTool(target);
            showToolDialog(m_groupSplit, [this]() { m_groupSplit->openSplitter(); });
        } else if (target == "SCORE_BOARD") {
            setActiveTool(target);
            showToolDialog(m_scoreBoard, [this]() { m_scoreBoard->openBoard(); });
        } else if (target == "AI_ASSISTANT") {
            setActiveTool(target);
            showToolDialog(m_aiAssistant, [this]() { m_aiAssistant->openAssistant(); });
        } else if (target == "SETTINGS") {
            setActiveTool(target);
            showToolDialog(m_settings, [this]() { openSettings(); });
        }
    }
}
