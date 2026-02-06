#pragma once

#include <QDialog>

class QLabel;
class QTimer;

class RandomNameDialog : public QDialog {
    Q_OBJECT
public:
    explicit RandomNameDialog(const QString &pickedName, QWidget *parent = nullptr);

private slots:
    void animate();

private:
    QLabel *m_nameLabel;
    QTimer *m_timer;
    int m_size = 18;
};
