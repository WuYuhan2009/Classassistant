#pragma once

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDialog>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QTimer>
#include <QTreeWidget>
#include <QWidget>

#include "../Utils.h"

class AttendanceSummaryWidget : public QWidget {
    Q_OBJECT
public:
    explicit AttendanceSummaryWidget(QWidget* parent = nullptr);
    void resetDaily();
    void applyAbsentees(const QStringList& absentees);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    QString m_lastResetDate;
    QStringList m_absentees;
    QLabel* m_title;
    QLabel* m_counts;
    QLabel* m_absentList;

    void syncDaily();
    void refreshUi();
};

class AttendanceSelectDialog : public QDialog {
    Q_OBJECT
public:
    explicit AttendanceSelectDialog(QWidget* parent = nullptr);
    void setSelectedAbsentees(const QStringList& absentees);

signals:
    void saved(const QStringList& absentees);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    QListWidget* m_roster;
    QLineEdit* m_searchEdit;
    void saveSelection();
    void exportSelection();
    void filterRoster(const QString& keyword);
};

class RandomCallDialog : public QDialog {
    Q_OBJECT
public:
    explicit RandomCallDialog(QWidget* parent = nullptr);
    void startAnim();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    QLabel* m_nameLabel;
    QLabel* m_hintLabel;
    QLabel* m_historyLabel;
    QPushButton* m_toggleButton;
    QPushButton* m_copyButton;
    QPushButton* m_closeButton;
    QTimer* m_timer;
    QStringList m_list;
    QStringList m_remainingList;
    QStringList m_history;
    int m_count = 0;
    bool m_running = false;

    void toggleRolling();
    QString drawName() const;
};

class ClassTimerDialog : public QDialog {
    Q_OBJECT
public:
    explicit ClassTimerDialog(QWidget* parent = nullptr);
    void openTimer();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    QLabel* m_countdownLabel;
    QSpinBox* m_minutesSpin;
    QPushButton* m_startPauseButton;
    QPushButton* m_resetButton;
    QTimer* m_timer;
    int m_remainingSeconds = 0;
    bool m_running = false;

    void updateCountdownText();
};

class ClassNoteDialog : public QDialog {
    Q_OBJECT
public:
    explicit ClassNoteDialog(QWidget* parent = nullptr);
    void openNote();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    QTextEdit* m_editor;
    QLabel* m_infoLabel;
    void saveNote();
};


class GroupSplitDialog : public QDialog {
    Q_OBJECT
public:
    explicit GroupSplitDialog(QWidget* parent = nullptr);
    void openSplitter();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    QSpinBox* m_groupSize;
    QTextEdit* m_result;
    void generate();
};

class ScoreBoardDialog : public QDialog {
    Q_OBJECT
public:
    explicit ScoreBoardDialog(QWidget* parent = nullptr);
    void openBoard();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    QLabel* m_teamALabel;
    QLabel* m_teamBLabel;
    QLabel* m_scoreLabel;
    int m_scoreA = 0;
    int m_scoreB = 0;
    void refreshScore();
};

class AddButtonDialog : public QDialog {
    Q_OBJECT
public:
    explicit AddButtonDialog(QWidget* parent = nullptr);
    AppButton resultButton() const;

private:
    QLineEdit* m_nameEdit;
    QLineEdit* m_iconEdit;
    QLineEdit* m_targetEdit;
    QComboBox* m_actionCombo;
};

class FirstRunWizard : public QDialog {
    Q_OBJECT
public:
    explicit FirstRunWizard(QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    QSlider* m_floatingOpacity;
    QSlider* m_summaryWidth;
    QCheckBox* m_startCollapsed;
    QCheckBox* m_trayClickToOpen;
    QCheckBox* m_showAttendanceSummaryOnStart;
    QCheckBox* m_randomNoRepeat;
    QCheckBox* m_allowExternalLinks;
    QLineEdit* m_seewoPathEdit;

    void finishSetup();
};

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);

signals:
    void configChanged();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    QTreeWidget* m_menuTree;
    QStackedWidget* m_stacked;

    QSlider* m_floatingOpacity;
    QSlider* m_summaryWidth;
    QCheckBox* m_startCollapsed;
    QCheckBox* m_trayClickToOpen;
    QCheckBox* m_showAttendanceSummaryOnStart;
    QCheckBox* m_compactMode;

    QCheckBox* m_randomNoRepeat;
    QSpinBox* m_historyCount;

    QCheckBox* m_allowExternalLinks;
    QSlider* m_animationDuration;
    QSlider* m_sidebarWidth;
    QCheckBox* m_collapseHidesToolWindows;
    QSpinBox* m_groupSize;
    QLineEdit* m_scoreTeamAName;
    QLineEdit* m_scoreTeamBName;
    QLineEdit* m_seewoPathEdit;
    QListWidget* m_buttonList;

    void loadData();
    void saveData();
    void importStudents();
    void addButton();
    void removeButton();
    void moveUp();
    void moveDown();
    void restoreDefaults();
    QWidget* createPageDisplayStartup();
    QWidget* createPageClassTools();
    QWidget* createPageDataManagement();
    QWidget* createPageSafety();
};
