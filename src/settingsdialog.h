#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include "buttonconfig.h"

namespace Ui { class SettingsDialog; }

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

private slots:
    void onApply();
    void onCancel();
    void browseSeewoPath();
    void importStudents();
    void addCustomButton();
    void removeCustomButton();
    void moveButtonUp();
    void moveButtonDown();
    void onButtonSelectionChanged();

private:
    Ui::SettingsDialog *ui;
    QList<ButtonConfig> m_buttons;

    void setupUI();
    void loadSettings();
    void saveSettings();
    void refreshButtonList();
};

#endif
