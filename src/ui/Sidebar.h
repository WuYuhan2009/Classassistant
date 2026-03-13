#pragma once

#include <QList>
#include <QTimer>
#include <QWidget>

#include "Tools.h"

class QPushButton;
class QEvent;
class QFocusEvent;
class QObject;

class Sidebar : public QWidget {
    Q_OBJECT
public:
    explicit Sidebar(QWidget* parent = nullptr);
    void rebuildUI();
    void openSettings();
    void hideAllToolWindowsAnimated();
    void triggerTool(const QString& target);
    void setAnchorGeometry(const QRect& anchorGeometry);
    bool isExpanded() const;
    void expandMenu();
    void collapseMenu();

signals:
    void requestCollapseToBall();

public slots:
    void reloadConfig();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private:
    AttendanceSummaryWidget* m_attendanceSummary;
    AttendanceSelectDialog* m_attendanceSelector;
    RandomCallDialog* m_randomCall;
    AIAssistantDialog* m_aiAssistant;
    SettingsDialog* m_settings;

    QList<QPushButton*> m_buttons;
    QTimer m_idleTimer;
    QRect m_anchorGeometry;
    bool m_suppressToolHideOnce = false;

    void handleAction(const QString& action, const QString& target);
    void handleFunctionAction(const QString& target);
    void launchExecutableTarget(const QString& target);
    void launchUrlTarget(const QString& target);
    QList<QWidget*> managedToolWindows() const;
    void showManagedWindow(QWidget* window);
    void refreshButtonLayout();
    void resetIdleCountdown();
    void onButtonTriggered(const QString& action, const QString& target);
};
