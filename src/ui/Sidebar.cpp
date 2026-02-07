#include "Sidebar.h"

#include "../Utils.h"

#include <QDesktopServices>
#include <QFile>
#include <QProcess>
#include <QPushButton>
#include <QScreen>
#include <QUrl>

Sidebar::Sidebar(QWidget* parent) : QWidget(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    m_attendance = new AttendanceWidget();
    m_randomCall = new RandomCallDialog();
    m_settings = new SettingsDialog();

    connect(m_settings, &SettingsDialog::configChanged, this, &Sidebar::rebuildUI);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(5, 20, 5, 20);
    m_layout->setSpacing(10);

    rebuildUI();
}

void Sidebar::rebuildUI() {
    QLayoutItem* item;
    while ((item = m_layout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    setStyleSheet("QWidget { background-color: rgba(240, 240, 240, 0.95); border-top-left-radius: 10px; border-bottom-left-radius: 10px; }");

    const auto buttons = Config::instance().getButtons();
    for (const auto& btnData : buttons) {
        auto* btn = new QPushButton(btnData.name.left(2));
        btn->setFixedSize(60, 60);
        btn->setToolTip(btnData.name);

        if (QFile::exists(btnData.iconPath)) {
            btn->setStyleSheet(QString("border-image: url(%1); border: none;").arg(btnData.iconPath));
            btn->setText("");
        } else {
            btn->setStyleSheet("background-color: white; border-radius: 10px; border: 1px solid #ccc; font-weight: bold; color: #333;");
        }

        connect(btn, &QPushButton::clicked, [this, btnData]() { handleAction(btnData.action, btnData.target); });
        m_layout->addWidget(btn);
    }

    m_layout->addStretch();

    auto* collapseBtn = new QPushButton(">>>");
    collapseBtn->setFixedSize(60, 40);
    collapseBtn->setStyleSheet("background-color: #ddd; border-radius: 5px;");
    connect(collapseBtn, &QPushButton::clicked, this, &Sidebar::requestHide);
    m_layout->addWidget(collapseBtn);
}

void Sidebar::handleAction(const QString& action, const QString& target) {
    if (action == "exe") {
        const QString path = (target == "SEEWO") ? Config::instance().seewoPath : target;
        if (!QProcess::startDetached(path)) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(path));
        }
    } else if (action == "url") {
        QDesktopServices::openUrl(QUrl(target));
    } else if (action == "func") {
        if (target == "ATTENDANCE") {
            if (m_attendance->isVisible()) {
                m_attendance->hide();
            } else {
                m_attendance->show();
            }
        } else if (target == "RANDOM_CALL") {
            m_randomCall->startAnim();
        } else if (target == "SETTINGS") {
            m_settings->exec();
        }
    }
}
