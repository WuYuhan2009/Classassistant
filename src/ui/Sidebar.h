#pragma once

#include <QVBoxLayout>
#include <QCloseEvent>
#include <QWidget>

#include "Tools.h"

class Sidebar : public QWidget {
    Q_OBJECT
public:
    explicit Sidebar(QWidget* parent = nullptr);
    void rebuildUI();
    void openSettings();

signals:
    void requestHide();

public slots:
    void reloadConfig();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    QVBoxLayout* m_layout;
    AttendanceWidget* m_attendance;
    RandomCallDialog* m_randomCall;
    SettingsDialog* m_settings;

    void handleAction(const QString& action, const QString& target);
};
