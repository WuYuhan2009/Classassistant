#pragma once

#include <QCloseEvent>
#include <QHash>
#include <QLabel>
#include <QTimer>
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
    QLabel* m_clockLabel;
    QHash<QString, QPushButton*> m_toolButtons;
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
    void updateClockText();
    void setActiveTool(const QString& target);
    QPushButton* createIconButton(const QString& text,
                                  const QString& iconPath,
                                  const QString& tooltip,
                                  const QString& fallbackEmoji = "");
};
