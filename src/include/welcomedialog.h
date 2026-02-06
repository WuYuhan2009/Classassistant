#pragma once

#include <QDialog>

class QComboBox;
class QSpinBox;

class WelcomeDialog : public QDialog {
    Q_OBJECT
public:
    explicit WelcomeDialog(QWidget *parent = nullptr);

    bool darkMode() const;
    int sidebarWidth() const;
    int iconSize() const;

private:
    QComboBox *m_themeCombo;
    QSpinBox *m_sidebarWidth;
    QSpinBox *m_iconSize;
};
