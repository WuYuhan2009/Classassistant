#pragma once

#include <QCloseEvent>
#include <QList>
#include <QVBoxLayout>
#include <QWidget>

#include "Tools.h"

class QPushButton;

class Sidebar : public QWidget {
    Q_OBJECT
public:
    explicit Sidebar(QWidget* parent = nullptr);
    void rebuildUI();
    void openSettings();
    void hideAllToolWindowsAnimated();
    void triggerTool(const QString& target);

signals:
    void requestHide();

public slots:
    void reloadConfig();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    QVBoxLayout* m_layout;
    AttendanceSummaryWidget* m_attendanceSummary;
    AttendanceSelectDialog* m_attendanceSelector;
    RandomCallDialog* m_randomCall;
    ClassTimerDialog* m_classTimer;
    ClassNoteDialog* m_classNote;
    GroupSplitDialog* m_groupSplit;
    ScoreBoardDialog* m_scoreBoard;
    AIAssistantDialog* m_aiAssistant;
    SettingsDialog* m_settings;

    void handleAction(const QString& action, const QString& target);
    void handleFunctionAction(const QString& target);
    void launchExecutableTarget(const QString& target);
    void launchUrlTarget(const QString& target);
    QList<QWidget*> managedToolWindows() const;
    void showManagedWindow(QWidget* window);

    QPushButton* createIconButton(const QString& text,
                                  const QString& iconPath,
                                  const QString& tooltip,
                                  const QString& fallbackEmoji = "");
};
