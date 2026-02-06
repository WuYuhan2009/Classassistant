#ifndef RANDOMNAMEDIALOG_H
#define RANDOMNAMEDIALOG_H

#include <QDialog>
#include <QStringList>

namespace Ui { class RandomNameDialog; }

class RandomNameDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RandomNameDialog(QWidget *parent = nullptr);
    ~RandomNameDialog();

private slots:
    void startRandom();
    void stopRandom();
    void updateRandomName();

private:
    Ui::RandomNameDialog *ui;
    QTimer *m_animationTimer;
    QStringList m_students;
    int m_currentIndex;
    int m_tickCount;

    void setupUI();
    void loadStudents();
};

#endif
