#pragma once

#include <QCloseEvent>
#include <QDialog>
#include <QListWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QTableWidget>
#include <QTimer>
#include <QWidget>

#include "../Utils.h"

class AttendanceWidget : public QWidget {
    Q_OBJECT
public:
    explicit AttendanceWidget(QWidget* parent = nullptr);
    void resetDaily();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onCellClicked(int row, int col);

private:
    QTableWidget* m_table;
    QLabel* m_summary;
    QString m_lastResetDate;

    void checkDailyReset();
    void updateSummary();
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
    QTimer* m_timer;
    QStringList m_list;
    int m_count = 0;
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
    QCheckBox* m_darkMode;
    QSlider* m_sidebarWidth;
    QSlider* m_iconSize;
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
    QCheckBox* m_darkMode;
    QSlider* m_sidebarWidth;
    QSlider* m_iconSize;
    QLineEdit* m_seewoPathEdit;
    QListWidget* m_buttonList;

    void loadData();
    void saveData();
    void importStudents();
    void addButton();
    void removeButton();
    void moveUp();
    void moveDown();
};
