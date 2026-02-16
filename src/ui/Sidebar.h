#pragma once

#include <QCloseEvent>
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
    SettingsDialog* m_settings;

    void handleAction(const QString& action, const QString& target);
    QPushButton* createIconButton(const QString& text,
                                  const QString& iconPath,
                                  const QString& tooltip,
                                  const QString& fallbackEmoji = "");
};
