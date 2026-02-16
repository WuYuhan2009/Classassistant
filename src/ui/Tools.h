#pragma once

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QSlider>
#include <QTimer>
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
    QSlider* m_floatingOpacity;
    QSlider* m_summaryWidth;
    QCheckBox* m_startCollapsed;
    QCheckBox* m_trayClickToOpen;
    QCheckBox* m_showAttendanceSummaryOnStart;
    QCheckBox* m_randomNoRepeat;
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
};
