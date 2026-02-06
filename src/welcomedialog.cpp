#include "welcomedialog.h"
#include "ui_welcomedialog.h"
#include "configmanager.h"

WelcomeDialog::WelcomeDialog(QWidget *parent) : QDialog(parent), ui(new Ui::WelcomeDialog) {
    ui->setupUi(this);
    setupUI();
    connect(ui->finishButton, &QPushButton::clicked, this, &WelcomeDialog::onFinish);
}

WelcomeDialog::~WelcomeDialog() { delete ui; }

void WelcomeDialog::setupUI() {
    ui->sidebarWidthSpinBox->setValue(220);
    ui->iconSizeSpinBox->setValue(48);
    ui->lightModeRadio->setChecked(true);
}

void WelcomeDialog::onFinish() {
    auto &cfg = ConfigManager::instance();
    cfg.setDarkMode(ui->darkModeRadio->isChecked());
    cfg.setSidebarWidth(ui->sidebarWidthSpinBox->value());
    cfg.setIconSize(ui->iconSizeSpinBox->value());
    cfg.save();
    accept();
}
