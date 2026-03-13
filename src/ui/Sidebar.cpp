#include "Sidebar.h"

#include "../Utils.h"
#include "FluentTheme.h"

#include <QApplication>
#include <QDesktopServices>
#include <QMessageBox>
#include <QMouseEvent>
#include <QProcess>
#include <QPushButton>
#include <QScreen>
#include <QSet>
#include <QUrl>
#include <QtMath>

namespace {
bool isAllowedTarget(const QString& target) {
    static const QSet<QString> kAllowedTargets = {"SEEWO", "ATTENDANCE", "RANDOM_CALL", "AI_ASSISTANT", "SETTINGS"};
    return kAllowedTargets.contains(target);
}
}

Sidebar::Sidebar(QWidget* parent) : QWidget(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_AcceptTouchEvents);

    m_attendanceSummary = new AttendanceSummaryWidget();
    if (Config::instance().showAttendanceSummaryOnStart) {
        m_attendanceSummary->show();
    }
    m_attendanceSelector = new AttendanceSelectDialog();
    m_randomCall = new RandomCallDialog();
    m_aiAssistant = new AIAssistantDialog();
    m_settings = new SettingsDialog();

    connect(m_settings, &SettingsDialog::configChanged, this, &Sidebar::reloadConfig);
    connect(m_attendanceSelector, &AttendanceSelectDialog::saved, m_attendanceSummary, &AttendanceSummaryWidget::applyAbsentees);

    m_idleTimer.setSingleShot(true);
    connect(&m_idleTimer, &QTimer::timeout, this, &Sidebar::collapseMenu);

    rebuildUI();
}

QList<QWidget*> Sidebar::managedToolWindows() const {
    return {m_attendanceSelector, m_randomCall, m_aiAssistant, m_settings};
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

void Sidebar::rebuildUI() {
    qDeleteAll(m_buttons);
    m_buttons.clear();

    const auto buttons = Config::instance().getButtons();
    bool hasSettings = false;
    for (const auto& b : buttons) {
        if (!isAllowedTarget(b.target)) {
            continue;
        }
        if (b.target == "SETTINGS") {
            hasSettings = true;
        }

        auto* btn = new QPushButton(this);
        btn->setToolTip(b.name);
        btn->setCursor(Qt::PointingHandCursor);
        const QIcon icon(Config::instance().resolveIconPath(b.iconPath));
        if (!icon.isNull()) {
            btn->setIcon(icon);
            btn->setIconSize(QSize(30, 30));
        } else {
            btn->setText(b.name.left(2));
        }

        connect(btn, &QPushButton::clicked, this, [this, b]() { onButtonTriggered(b.action, b.target); });
        m_buttons.push_back(btn);
    }

    if (!hasSettings) {
        auto* settingsBtn = new QPushButton(this);
        settingsBtn->setToolTip("设置");
        const QIcon settingsIcon(Config::instance().resolveIconPath("icon_settings.png"));
        if (!settingsIcon.isNull()) {
            settingsBtn->setIcon(settingsIcon);
            settingsBtn->setIconSize(QSize(30, 30));
        } else {
            settingsBtn->setText("设");
        }
        connect(settingsBtn, &QPushButton::clicked, this, [this]() { onButtonTriggered("func", "SETTINGS"); });
        m_buttons.push_back(settingsBtn);
    }

    refreshButtonLayout();
}

void Sidebar::openSettings() {
    showManagedWindow(m_settings);
}

void Sidebar::triggerTool(const QString& target) {
    if (!isAllowedTarget(target)) {
        return;
    }
    handleAction("func", target);
}

void Sidebar::hideAllToolWindowsAnimated() {
    if (!Config::instance().collapseHidesToolWindows) {
        return;
    }

    for (QWidget* window : managedToolWindows()) {
        if (window && window->isVisible()) {
            window->hide();
            window->setWindowOpacity(1.0);
        }
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
    } else if (target == "AI_ASSISTANT") {
        m_aiAssistant->setWindowOpacity(1.0);
        m_aiAssistant->openAssistant();
    } else if (target == "SETTINGS") {
        openSettings();
    }
}

void Sidebar::handleAction(const QString& action, const QString& target) {
    if (!isAllowedTarget(target)) {
        Logger::instance().warn(QString("忽略未允许目标: %1").arg(target));
        return;
    }

    Logger::instance().info(QString("触发按钮 action=%1 target=%2").arg(action, target));
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

void Sidebar::setAnchorGeometry(const QRect& anchorGeometry) {
    m_anchorGeometry = anchorGeometry;
    refreshButtonLayout();
}

bool Sidebar::isExpanded() const {
    return isVisible();
}

void Sidebar::expandMenu() {
    const QRect screen = QApplication::primaryScreen()->availableGeometry();
    setGeometry(screen);
    show();
    raise();
    activateWindow();
    refreshButtonLayout();
    resetIdleCountdown();
    Logger::instance().info("菜单展开");
}

void Sidebar::collapseMenu() {
    if (!isVisible()) {
        return;
    }
    m_idleTimer.stop();
    hideAllToolWindowsAnimated();
    hide();
    emit requestCollapseToBall();
    Logger::instance().info("菜单收起");
}

void Sidebar::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    refreshButtonLayout();
}

void Sidebar::mousePressEvent(QMouseEvent* event) {
    QWidget* child = childAt(event->pos());
    if (qobject_cast<QPushButton*>(child) != nullptr) {
        QWidget::mousePressEvent(event);
        return;
    }
    collapseMenu();
    QWidget::mousePressEvent(event);
}

void Sidebar::resetIdleCountdown() {
    m_idleTimer.start(Config::instance().menuAutoCollapseSeconds * 1000);
}

void Sidebar::onButtonTriggered(const QString& action, const QString& target) {
    handleAction(action, target);
    collapseMenu();
}

void Sidebar::refreshButtonLayout() {
    if (m_anchorGeometry.isNull()) {
        return;
    }

    const int btnSize = qMax(56, Config::instance().floatingBallSize - 4);
    const QString btnStyle = QString("QPushButton{background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(255,255,255,245),stop:1 rgba(235,242,252,238));"
                                    "border:1px solid #c6d4e8;border-radius:%1px;font-size:14px;font-weight:700;color:#1f3550;}"
                                    "QPushButton:hover{background:#edf5ff;border-color:#9eb9da;}"
                                    "QPushButton:pressed{background:#deebfb;border-color:#84a4cc;}")
                                .arg(btnSize / 2);

    for (auto* btn : m_buttons) {
        btn->setFixedSize(btnSize, btnSize);
        btn->setStyleSheet(btnStyle);
        btn->hide();
    }

    const QPoint center = m_anchorGeometry.center() - geometry().topLeft();
    const int count = m_buttons.size();
    if (count <= 0) {
        return;
    }

    const bool anchorAtRight = center.x() >= width() / 2;
    const qreal startDeg = anchorAtRight ? 120.0 : -60.0;
    const qreal endDeg = anchorAtRight ? 240.0 : 60.0;

    const int minGap = 12;
    const qreal arcDeg = qAbs(endDeg - startDeg);
    const qreal stepDeg = count <= 1 ? arcDeg : arcDeg / (count - 1);
    const qreal stepRad = qDegreesToRadians(stepDeg);
    const int requiredRadius = count <= 1 ? Config::instance().radialMenuRadius
                                          : qCeil((btnSize + minGap) / qMax(0.20, stepRad));

    const int screenPad = btnSize / 2 + 8;
    const int maxRadius = anchorAtRight ? (center.x() - screenPad) : ((width() - center.x()) - screenPad);
    const int radius = qBound(requiredRadius, Config::instance().radialMenuRadius, qMax(requiredRadius, maxRadius));

    for (int i = 0; i < count; ++i) {
        const qreal t = count <= 1 ? 0.5 : static_cast<qreal>(i) / (count - 1);
        const qreal deg = startDeg + (endDeg - startDeg) * t;
        const qreal rad = qDegreesToRadians(deg);
        const int x = center.x() + qRound(radius * qCos(rad)) - btnSize / 2;
        const int y = center.y() + qRound(radius * qSin(rad)) - btnSize / 2;

        m_buttons[i]->move(x, y);
        m_buttons[i]->show();
        m_buttons[i]->raise();
    }
}
