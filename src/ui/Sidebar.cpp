#include "Sidebar.h"

#include "../Utils.h"
#include "FluentTheme.h"

#include <QApplication>
#include <QDesktopServices>
#include <QEvent>
#include <QFocusEvent>
#include <QMap>
#include <QMessageBox>
#include <QMouseEvent>
#include <QParallelAnimationGroup>
#include <QProcess>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QToolButton>
#include <QScreen>
#include <QSet>
#include <QTime>
#include <QUrl>
#include <QtMath>
#include <functional>

namespace {
const QStringList kOrderedTargets = {"SEEWO", "ATTENDANCE", "SCREEN_OFF", "RANDOM_CALL", "AI_ASSISTANT", "SETTINGS"};

bool isAllowedTarget(const QString& target) {
    return kOrderedTargets.contains(target);
}

int orderIndex(const QString& target) {
    const int idx = kOrderedTargets.indexOf(target);
    return idx < 0 ? 999 : idx;
}

bool inSelfStudyPeriod() {
    const QTime now = QTime::currentTime();
    for (const QString& period : Config::instance().selfStudyPeriods) {
        const QStringList parts = period.split('-', Qt::SkipEmptyParts);
        if (parts.size() != 2) continue;
        const QTime s = QTime::fromString(parts[0].trimmed(), "HH:mm");
        const QTime e = QTime::fromString(parts[1].trimmed(), "HH:mm");
        if (!s.isValid() || !e.isValid()) continue;
        if (now >= s && now <= e) return true;
    }
    return false;
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
    m_screenOff = new ScreenOffOverlay();
    connect(m_screenOff, &ScreenOffOverlay::exited, this, [this]() {
        m_attendanceSummary->setPinnedOnTop(false);
    });

    connect(m_settings, &SettingsDialog::configChanged, this, &Sidebar::reloadConfig);
    connect(m_attendanceSelector, &AttendanceSelectDialog::saved, m_attendanceSummary, &AttendanceSummaryWidget::applyAbsentees);

    m_idleTimer.setSingleShot(true);
    connect(&m_idleTimer, &QTimer::timeout, this, &Sidebar::collapseMenu);

    qApp->installEventFilter(this);
    rebuildUI();
}

QList<QWidget*> Sidebar::managedToolWindows() const {
    return {m_attendanceSelector, m_randomCall, m_aiAssistant, m_settings, m_screenOff};
}

void Sidebar::showManagedWindow(QWidget* window) {
    if (!window) return;
    window->setWindowOpacity(1.0);
    window->show();
    window->raise();
    window->activateWindow();
}

void Sidebar::rebuildUI() {
    qDeleteAll(m_buttons);
    m_buttons.clear();
    m_buttonExpandedPos.clear();

    const auto buttons = Config::instance().getButtons();
    QMap<int, AppButton> ordered;
    for (const auto& b : buttons) {
        if (!isAllowedTarget(b.target)) continue;
        ordered.insert(orderIndex(b.target), b);
    }
    if (!ordered.contains(orderIndex("SETTINGS"))) {
        ordered.insert(orderIndex("SETTINGS"), {"设置", "icon_settings.svg", "func", "SETTINGS", true});
    }

    for (const auto& b : ordered) {
        auto* btn = new QToolButton(this);
        btn->setToolTip(b.name);
        btn->setCursor(Qt::PointingHandCursor);
        const QIcon icon(Config::instance().resolveIconPath(b.iconPath));
        btn->setText(b.name);
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        if (!icon.isNull()) {
            btn->setIcon(icon);
            btn->setIconSize(QSize(qMax(20, Config::instance().iconSize - 12), qMax(20, Config::instance().iconSize - 12)));
        }
        connect(btn, &QToolButton::clicked, this, [this, b]() { onButtonTriggered(b.action, b.target); });
        m_buttons.push_back(btn);
    }

    refreshButtonLayout();
}

void Sidebar::openSettings() { showManagedWindow(m_settings); }

void Sidebar::triggerTool(const QString& target) {
    if (!isAllowedTarget(target)) return;
    handleAction("func", target);
}

void Sidebar::hideAllToolWindowsAnimated() {
    if (!Config::instance().collapseHidesToolWindows) return;
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
    if (Config::instance().showAttendanceSummaryOnStart) m_attendanceSummary->show();
    else m_attendanceSummary->hide();
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
    } else if (target == "SCREEN_OFF") {
        if (m_screenOff->isActive()) {
            m_screenOff->deactivate();
            m_attendanceSummary->setPinnedOnTop(false);
            QTimer::singleShot(80, this, [this]() { m_attendanceSummary->setPinnedOnTop(false); });
            return;
        }

        m_screenOff->activate(inSelfStudyPeriod());
        for (int delayMs = 2000; delayMs <= 20000; delayMs += 2000) {
            QTimer::singleShot(delayMs, this, [this]() {
                if (!m_screenOff->isActive()) {
                    return;
                }
                m_attendanceSummary->setPinnedOnTop(true);
                m_attendanceSummary->show();
                m_attendanceSummary->raise();
                m_attendanceSummary->activateWindow();
            });
        }
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
    if (action == "exe") launchExecutableTarget(target);
    else if (action == "url") launchUrlTarget(target);
    else if (action == "func") handleFunctionAction(target);
}

void Sidebar::setAnchorGeometry(const QRect& anchorGeometry) {
    m_anchorGeometry = anchorGeometry;
    refreshButtonLayout();
}

bool Sidebar::isExpanded() const { return isVisible(); }

void Sidebar::expandMenu() {
    const QRect screen = QApplication::primaryScreen()->availableGeometry();
    setGeometry(screen);
    refreshButtonLayout();
    show();
    raise();
    activateWindow();
    animateButtons(true);
    resetIdleCountdown();
    Logger::instance().info("菜单展开");
}

void Sidebar::collapseMenu() {
    if (!isVisible() || m_isAnimating) return;
    m_idleTimer.stop();
    animateButtons(false);
}

void Sidebar::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    refreshButtonLayout();
}

void Sidebar::mousePressEvent(QMouseEvent* event) {
    QWidget* child = childAt(event->pos());
    if (qobject_cast<QToolButton*>(child) == nullptr) {
        collapseMenu();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

bool Sidebar::eventFilter(QObject* watched, QEvent* event) {
    Q_UNUSED(watched);
    if (!isVisible() || m_isAnimating) return QWidget::eventFilter(watched, event);

    if (event->type() == QEvent::MouseButtonPress) {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        const QPoint localPos = mapFromGlobal(mouseEvent->globalPos());
        for (auto* btn : m_buttons) {
            if (btn->isVisible() && btn->geometry().contains(localPos)) {
                return QWidget::eventFilter(watched, event);
            }
        }
        collapseMenu();
    }
    return QWidget::eventFilter(watched, event);
}

void Sidebar::focusOutEvent(QFocusEvent* event) {
    QWidget::focusOutEvent(event);
    if (isVisible()) collapseMenu();
}

void Sidebar::resetIdleCountdown() {
    m_idleTimer.start(Config::instance().menuAutoCollapseSeconds * 1000);
}

void Sidebar::onButtonTriggered(const QString& action, const QString& target) {
    handleAction(action, target);
    m_suppressToolHideOnce = true;
    collapseMenu();
}

void Sidebar::refreshButtonLayout() {
    if (m_anchorGeometry.isNull()) return;

    const int btnSize = qMax(88, Config::instance().floatingBallSize + 20);
    const QString btnStyle = QString("QToolButton{background:#ffffff;border:1px solid #d9d9d9;border-radius:%1px;font-size:12px;font-weight:600;color:#262626;padding:4px 6px;}"
                                    "QToolButton:hover{background:#f5f5f5;border-color:#91caff;}"
                                    "QToolButton:pressed{background:#e6f4ff;border-color:#1677ff;}")
                                .arg(14);

    const QPoint center = m_anchorGeometry.center() - geometry().topLeft();
    const int count = m_buttons.size();
    if (count <= 0) return;

    const bool anchorAtRight = center.x() >= width() / 2;
    const qreal startDeg = anchorAtRight ? 90.0 : -90.0;
    const qreal endDeg = anchorAtRight ? 270.0 : 90.0;

    const qreal arcDeg = qAbs(endDeg - startDeg);
    const qreal stepDeg = count <= 1 ? arcDeg : arcDeg / (count - 1);
    const qreal stepRad = qDegreesToRadians(stepDeg);
    const int desiredSpacing = btnSize + 8;
    const int minRadiusForNoOverlap = count <= 1 ? btnSize : qCeil(desiredSpacing / qMax(0.2, 2.0 * qSin(stepRad / 2.0)));

    const int maxPreferredRadius = btnSize * 2;
    const int targetByConfig = qMin(Config::instance().radialMenuRadius, maxPreferredRadius);
    const int margin = btnSize / 2 + 10;
    const int horizontalLimit = anchorAtRight ? (center.x() - margin) : (width() - center.x() - margin);
    const int verticalLimit = qMin(center.y() - margin, height() - center.y() - margin);
    const int safeMaxRadius = qMax(minRadiusForNoOverlap, qMin(horizontalLimit, verticalLimit));
    const int radius = qBound(minRadiusForNoOverlap, targetByConfig, safeMaxRadius);

    m_buttonExpandedPos.clear();
    for (int i = 0; i < count; ++i) {
        qreal t = count <= 1 ? 0.5 : static_cast<qreal>(i) / (count - 1);
        if (anchorAtRight) t = 1.0 - t;
        const qreal deg = startDeg + (endDeg - startDeg) * t;
        const qreal rad = qDegreesToRadians(deg);
        const int x = center.x() + qRound(radius * qCos(rad)) - btnSize / 2;
        const int y = center.y() + qRound(radius * qSin(rad)) - btnSize / 2;

        auto* btn = m_buttons[i];
        btn->setFixedSize(btnSize, btnSize);
        btn->setStyleSheet(btnStyle);
        m_buttonExpandedPos.insert(btn, QPoint(x, y));
    }
}

void Sidebar::animateButtons(bool expanding) {
    if (m_anchorGeometry.isNull() || m_buttons.isEmpty()) return;

    m_isAnimating = true;
    const QPoint center = m_anchorGeometry.center() - geometry().topLeft() - QPoint(m_buttons.first()->width() / 2, m_buttons.first()->height() / 2);

    auto* group = new QParallelAnimationGroup(this);
    for (auto* btn : m_buttons) {
        const QPoint endPos = m_buttonExpandedPos.value(btn, center);
        const QPoint startPos = center;

        auto* posAnim = new QPropertyAnimation(btn, "pos", group);
        posAnim->setDuration(220);
        posAnim->setEasingCurve(expanding ? QEasingCurve::OutCubic : QEasingCurve::InCubic);
        posAnim->setStartValue(expanding ? startPos : endPos);
        posAnim->setEndValue(expanding ? endPos : startPos);

        auto* opaAnim = new QPropertyAnimation(btn, "windowOpacity", group);
        opaAnim->setDuration(200);
        opaAnim->setStartValue(expanding ? 0.0 : 1.0);
        opaAnim->setEndValue(expanding ? 1.0 : 0.0);

        if (expanding) {
            btn->move(startPos);
            btn->setWindowOpacity(0.0);
            btn->show();
            btn->raise();
        }
        group->addAnimation(posAnim);
        group->addAnimation(opaAnim);
    }

    connect(group, &QParallelAnimationGroup::finished, this, [this, expanding]() {
        m_isAnimating = false;
        if (!expanding) {
            for (auto* btn : m_buttons) {
                btn->hide();
                btn->setWindowOpacity(1.0);
            }
            if (!m_suppressToolHideOnce) hideAllToolWindowsAnimated();
            m_suppressToolHideOnce = false;
            hide();
            emit requestCollapseToBall();
            Logger::instance().info("菜单收起");
        }
    });
    group->start(QAbstractAnimation::DeleteWhenStopped);
}
