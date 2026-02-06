#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "configmanager.h"
#include "settingsdialog.h"
#include "attendancewidget.h"
#include "randomnamedialog.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDesktopServices>
#include <QProcess>
#include <QApplication>
#include <QScreen>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), m_trayIcon(nullptr), m_trayMenu(nullptr), m_buttonLayout(nullptr) {
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setupUI();
    setupTray();
    createButtons();
    applyTheme();
    positionSidebar();
    hide();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::setupUI() {
    setFixedWidth(ConfigManager::instance().sidebarWidth());
    QRect r = QApplication::primaryScreen()->availableGeometry();
    setFixedHeight(int(r.height() * 0.7));
    QWidget *container = new QWidget(this);
    m_buttonLayout = new QVBoxLayout(container);
    m_buttonLayout->addStretch();
    setCentralWidget(container);
}

void MainWindow::setupTray() {
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(QIcon(":/icons/app.ico"));
    m_trayMenu = new QMenu(this);
    m_trayMenu->addAction("显示侧边栏", this, &MainWindow::showSidebar);
    m_trayMenu->addAction("设置", this, &MainWindow::showSettings);
    m_trayMenu->addAction("重载配置", this, &MainWindow::reloadConfig);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction("退出", this, &MainWindow::quitApplication);
    m_trayIcon->setContextMenu(m_trayMenu);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayActivated);
    m_trayIcon->show();
}

void MainWindow::createButtons() {
    QLayoutItem *item;
    while ((item = m_buttonLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    const auto buttons = ConfigManager::instance().getButtons();
    int iconSize = ConfigManager::instance().iconSize();
    for (const auto &b : buttons) {
        if (!b.enabled) continue;
        auto *btn = new QPushButton(b.name, this);
        btn->setIcon(QIcon(QString(":/icons/%1.svg").arg(b.icon)));
        btn->setIconSize(QSize(iconSize, iconSize));
        btn->setProperty("buttonConfig", QVariant::fromValue(b));
        connect(btn, &QPushButton::clicked, this, &MainWindow::onButtonClicked);
        m_buttonLayout->addWidget(btn);
    }
    m_buttonLayout->addStretch();
}

void MainWindow::applyTheme() {
    if (ConfigManager::instance().isDarkMode()) {
        setStyleSheet("QMainWindow{background:rgba(30,30,30,230);} QPushButton{background:rgba(50,50,50,200);color:white;border-radius:8px;padding:10px;text-align:left;} QPushButton:hover{background:rgba(70,130,255,200);}");
    } else {
        setStyleSheet("QMainWindow{background:rgba(240,240,240,230);} QPushButton{background:rgba(255,255,255,200);color:#333;border-radius:8px;padding:10px;text-align:left;} QPushButton:hover{background:rgba(70,130,255,150);color:white;}");
    }
}

void MainWindow::positionSidebar() {
    QRect r = QApplication::primaryScreen()->availableGeometry();
    move(r.right() - width() - 10, (r.height() - height()) / 2);
}

void MainWindow::showSidebar() { positionSidebar(); show(); raise(); }
void MainWindow::hideSidebar() { hide(); emit sidebarHidden(); }
void MainWindow::closeEvent(QCloseEvent *event) { event->ignore(); hideSidebar(); }

void MainWindow::onButtonClicked() {
    auto *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    auto var = btn->property("buttonConfig");
    if (!var.isValid()) return;
    executeButton(var.value<ButtonConfig>());
    hideSidebar();
}

void MainWindow::executeButton(const ButtonConfig &config) {
    if (config.type == "program") {
        const QString path = config.data.isEmpty() ? ConfigManager::instance().getSeewoPath() : config.data;
        if (path.isEmpty()) { QMessageBox::warning(this, "错误", "未设置程序路径"); return; }
        QProcess::startDetached(path);
    } else if (config.type == "url") {
        QDesktopServices::openUrl(QUrl(config.data));
    } else if (config.type == "attendance") {
        static AttendanceWidget *w = nullptr; if (!w) w = new AttendanceWidget(); w->show(); w->raise();
    } else if (config.type == "random") {
        RandomNameDialog(this).exec();
    } else if (config.type == "classisland") {
#ifdef Q_OS_WIN
        QDesktopServices::openUrl(QUrl("classisland://app/schedule/change"));
#else
        QMessageBox::information(this, "提示", "Linux 下暂不支持 ClassIsland 集成。\n请手动使用 ClassIsland 更换课表。");
#endif
    } else if (config.type == "settings") {
        showSettings();
    }
}

void MainWindow::showSettings() { SettingsDialog dlg(this); if (dlg.exec() == QDialog::Accepted) reloadConfig(); }
void MainWindow::reloadConfig() { ConfigManager::instance().load(); createButtons(); applyTheme(); }
void MainWindow::quitApplication() { ConfigManager::instance().save(); qApp->quit(); }
void MainWindow::onTrayActivated(QSystemTrayIcon::ActivationReason reason) { if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) showSidebar(); }
