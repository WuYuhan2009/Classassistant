#ifndef ATTENDANCEWIDGET_H
#define ATTENDANCEWIDGET_H

#include <QWidget>
#include <QStringList>

class QListWidget;
class QListWidgetItem;
class QLabel;

class AttendanceWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AttendanceWidget(QWidget *parent = nullptr);

private slots:
    void onStudentClicked(QListWidgetItem *item);
    void resetDaily();

private:
    QLabel *m_totalLabel;
    QLabel *m_presentLabel;
    QLabel *m_absentLabel;
    QListWidget *m_studentList;
    QStringList m_allStudents;
    QStringList m_absentStudents;

    void setupUI();
    void loadStudents();
    void updateDisplay();
    void positionWindow();
    void startDailyTimer();
};

#endif
