#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include "buttonconfig.h"

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void showSidebar();
    void hideSidebar();

signals:
    void sidebarHidden();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onButtonClicked();
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void showSettings();
    void reloadConfig();
    void quitApplication();

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
    QVBoxLayout *m_buttonLayout;

    void setupUI();
    void setupTray();
    void createButtons();
    void applyTheme();
    void positionSidebar();
    void executeButton(const ButtonConfig &config);
};

#endif
