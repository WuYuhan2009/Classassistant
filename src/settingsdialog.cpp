#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "configmanager.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QInputDialog>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent), ui(new Ui::SettingsDialog) {
    ui->setupUi(this);
    setupUI();
    loadSettings();
    connect(ui->applyButton, &QPushButton::clicked, this, &SettingsDialog::onApply);
    connect(ui->cancelButton, &QPushButton::clicked, this, &SettingsDialog::onCancel);
    connect(ui->browseSeewoButton, &QPushButton::clicked, this, &SettingsDialog::browseSeewoPath);
    connect(ui->importStudentsButton, &QPushButton::clicked, this, &SettingsDialog::importStudents);
    connect(ui->addButtonButton, &QPushButton::clicked, this, &SettingsDialog::addCustomButton);
    connect(ui->removeButtonButton, &QPushButton::clicked, this, &SettingsDialog::removeCustomButton);
    connect(ui->moveUpButton, &QPushButton::clicked, this, &SettingsDialog::moveButtonUp);
    connect(ui->moveDownButton, &QPushButton::clicked, this, &SettingsDialog::moveButtonDown);
    connect(ui->buttonListWidget, &QListWidget::itemSelectionChanged, this, &SettingsDialog::onButtonSelectionChanged);
}

SettingsDialog::~SettingsDialog() { delete ui; }

void SettingsDialog::setupUI() {
    ui->removeButtonButton->setEnabled(false);
    ui->moveUpButton->setEnabled(false);
    ui->moveDownButton->setEnabled(false);
}

void SettingsDialog::loadSettings() {
    auto &cfg = ConfigManager::instance();
    ui->darkModeCheckBox->setChecked(cfg.isDarkMode());
    ui->sidebarWidthSpinBox->setValue(cfg.sidebarWidth());
    ui->iconSizeSpinBox->setValue(cfg.iconSize());
    ui->seewoPathLineEdit->setText(cfg.getSeewoPath());
    ui->studentCountLabel->setText(QString("当前名单: %1 人").arg(cfg.getStudents().size()));
    m_buttons = cfg.getButtons();
    refreshButtonList();
}

void SettingsDialog::saveSettings() {
    auto &cfg = ConfigManager::instance();
    cfg.setDarkMode(ui->darkModeCheckBox->isChecked());
    cfg.setSidebarWidth(ui->sidebarWidthSpinBox->value());
    cfg.setIconSize(ui->iconSizeSpinBox->value());
    cfg.setSeewoPath(ui->seewoPathLineEdit->text());
    cfg.setButtons(m_buttons);
    cfg.save();
}

void SettingsDialog::refreshButtonList() {
    ui->buttonListWidget->clear();
    for (int i=0;i<m_buttons.size();++i) {
        QString text = QString("%1. %2").arg(i+1).arg(m_buttons[i].name);
        if (m_buttons[i].isDefault) text += " [默认]";
        auto *item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, i);
        ui->buttonListWidget->addItem(item);
    }
}

void SettingsDialog::onApply() { saveSettings(); accept(); }
void SettingsDialog::onCancel() { reject(); }

void SettingsDialog::browseSeewoPath() {
    QString path = QFileDialog::getOpenFileName(this, "选择希沃白板程序", "", "可执行文件 (*.exe);;快捷方式 (*.lnk);;所有文件 (*)");
    if (!path.isEmpty()) ui->seewoPathLineEdit->setText(path);
}

void SettingsDialog::importStudents() {
    QString fileName = QFileDialog::getOpenFileName(this, "导入学生名单", "", "文本文件 (*.txt);;CSV文件 (*.csv);;Excel文件 (*.xlsx *.xls);;所有文件 (*)");
    if (fileName.isEmpty()) return;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QTextStream in(&file);
    in.setCodec("UTF-8");
    QStringList students;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        if (fileName.endsWith(".csv", Qt::CaseInsensitive)) {
            students << line.split(',').value(0).trimmed();
        } else {
            students << line;
        }
    }
    ConfigManager::instance().setStudents(students);
    ui->studentCountLabel->setText(QString("当前名单: %1 人").arg(students.size()));
}

void SettingsDialog::addCustomButton() {
    bool ok;
    QString name = QInputDialog::getText(this, "添加自定义按钮", "按钮名称:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return;
    QString type = QInputDialog::getItem(this, "添加自定义按钮", "按钮类型:", {"打开程序","打开网址"}, 0, false, &ok);
    if (!ok) return;
    QString data;
    if (type == "打开程序") {
        data = QFileDialog::getOpenFileName(this, "选择程序", "", "可执行文件 (*.exe);;所有文件 (*)");
    } else {
        data = QInputDialog::getText(this, "添加自定义按钮", "网址:", QLineEdit::Normal, "https://", &ok);
    }
    if (data.isEmpty()) return;
    ButtonConfig b; b.name=name; b.icon="custom"; b.type=(type=="打开程序")?"program":"url"; b.data=data; b.enabled=true; b.isDefault=false;
    m_buttons.append(b);
    refreshButtonList();
}

void SettingsDialog::removeCustomButton() {
    auto *item = ui->buttonListWidget->currentItem(); if (!item) return;
    int index = item->data(Qt::UserRole).toInt();
    if (m_buttons[index].isDefault) { QMessageBox::warning(this, "提示", "默认按钮不可删除！"); return; }
    m_buttons.removeAt(index);
    refreshButtonList();
}

void SettingsDialog::moveButtonUp() { auto *item = ui->buttonListWidget->currentItem(); if (!item) return; int i=item->data(Qt::UserRole).toInt(); if (i>0){m_buttons.move(i,i-1); refreshButtonList(); ui->buttonListWidget->setCurrentRow(i-1);} }
void SettingsDialog::moveButtonDown() { auto *item = ui->buttonListWidget->currentItem(); if (!item) return; int i=item->data(Qt::UserRole).toInt(); if (i<m_buttons.size()-1){m_buttons.move(i,i+1); refreshButtonList(); ui->buttonListWidget->setCurrentRow(i+1);} }

void SettingsDialog::onButtonSelectionChanged() {
    auto *item = ui->buttonListWidget->currentItem();
    if (!item) { ui->removeButtonButton->setEnabled(false); ui->moveUpButton->setEnabled(false); ui->moveDownButton->setEnabled(false); return; }
    int i = item->data(Qt::UserRole).toInt();
    ui->removeButtonButton->setEnabled(!m_buttons[i].isDefault);
    ui->moveUpButton->setEnabled(i > 0);
    ui->moveDownButton->setEnabled(i < m_buttons.size() - 1);
}
