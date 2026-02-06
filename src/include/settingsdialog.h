#pragma once

#include <QDialog>

class ConfigManager;
class QListWidget;
class QComboBox;
class QSpinBox;
class QLineEdit;

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(ConfigManager *config, QWidget *parent = nullptr);

private slots:
    void importClassList();
    void addCustomButton();
    void removeSelectedButton();
    void moveUp();
    void moveDown();
    void chooseWhiteboardPath();
    void saveAndClose();

private:
    void loadFromConfig();
    void syncButtonsToConfig();

    ConfigManager *m_config;
    QComboBox *m_themeCombo;
    QSpinBox *m_sidebarWidth;
    QSpinBox *m_iconSize;
    QLineEdit *m_whiteboardPath;
    QListWidget *m_buttonsList;
};
