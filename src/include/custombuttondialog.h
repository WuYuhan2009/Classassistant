#pragma once

#include <QDialog>
#include "models.h"

class QLineEdit;
class QComboBox;

class CustomButtonDialog : public QDialog {
    Q_OBJECT
public:
    explicit CustomButtonDialog(QWidget *parent = nullptr);
    ButtonAction action() const;

private slots:
    void browseTarget();
    void browseIcon();

private:
    QLineEdit *m_name;
    QLineEdit *m_target;
    QLineEdit *m_icon;
    QComboBox *m_type;
};
