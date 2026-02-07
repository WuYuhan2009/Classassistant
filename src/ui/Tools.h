#pragma once

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QTableWidget>
#include <QTimer>
#include <QWidget>

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
    void updateSummary();
};

class RandomCallDialog : public QDialog {
    Q_OBJECT
public:
    explicit RandomCallDialog(QWidget* parent = nullptr);
    void startAnim();

private:
    QLabel* m_nameLabel;
    QTimer* m_timer;
    QStringList m_list;
    int m_count = 0;
};

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);

signals:
    void configChanged();

private:
    QLineEdit* m_seewoPathEdit;
    void saveData();
    void importStudents();
};
