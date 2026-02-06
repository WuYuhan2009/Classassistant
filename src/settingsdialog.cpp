#include "settingsdialog.h"

#include "configmanager.h"
#include "custombuttondialog.h"

#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTextStream>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLineEdit>

SettingsDialog::SettingsDialog(ConfigManager *config, QWidget *parent)
    : QDialog(parent), m_config(config) {
    setWindowTitle("Classassistant 设置");
    resize(700, 560);

    auto *root = new QVBoxLayout(this);

    m_themeCombo = new QComboBox;
    m_themeCombo->addItems({"深色", "浅色"});
    m_sidebarWidth = new QSpinBox;
    m_sidebarWidth->setRange(72, 180);
    m_iconSize = new QSpinBox;
    m_iconSize->setRange(20, 64);
    m_whiteboardPath = new QLineEdit;

    auto *pathBtn = new QPushButton("选择希沃白板路径");
    connect(pathBtn, &QPushButton::clicked, this, &SettingsDialog::chooseWhiteboardPath);

    root->addWidget(new QLabel("主题"));
    root->addWidget(m_themeCombo);
    root->addWidget(new QLabel("侧栏宽度"));
    root->addWidget(m_sidebarWidth);
    root->addWidget(new QLabel("图标大小"));
    root->addWidget(m_iconSize);
    root->addWidget(new QLabel("希沃白板路径（exe 或快捷方式）"));
    root->addWidget(m_whiteboardPath);
    root->addWidget(pathBtn);

    auto *importBtn = new QPushButton("导入班级名单 (Excel/CSV/TXT)");
    connect(importBtn, &QPushButton::clicked, this, &SettingsDialog::importClassList);
    root->addWidget(importBtn);

    root->addWidget(new QLabel("按钮管理"));
    m_buttonsList = new QListWidget;
    root->addWidget(m_buttonsList, 1);

    auto *btnRow = new QHBoxLayout;
    auto *addBtn = new QPushButton("添加");
    auto *removeBtn = new QPushButton("删除");
    auto *upBtn = new QPushButton("上移");
    auto *downBtn = new QPushButton("下移");
    connect(addBtn, &QPushButton::clicked, this, &SettingsDialog::addCustomButton);
    connect(removeBtn, &QPushButton::clicked, this, &SettingsDialog::removeSelectedButton);
    connect(upBtn, &QPushButton::clicked, this, &SettingsDialog::moveUp);
    connect(downBtn, &QPushButton::clicked, this, &SettingsDialog::moveDown);
    btnRow->addWidget(addBtn);
    btnRow->addWidget(removeBtn);
    btnRow->addWidget(upBtn);
    btnRow->addWidget(downBtn);
    root->addLayout(btnRow);

    auto *saveBtn = new QPushButton("保存");
    connect(saveBtn, &QPushButton::clicked, this, &SettingsDialog::saveAndClose);
    root->addWidget(saveBtn);

    loadFromConfig();
}

void SettingsDialog::loadFromConfig() {
    const auto &cfg = m_config->config();
    m_themeCombo->setCurrentIndex(cfg.theme.darkMode ? 0 : 1);
    m_sidebarWidth->setValue(cfg.theme.sidebarWidth);
    m_iconSize->setValue(cfg.theme.iconSize);
    m_whiteboardPath->setText(cfg.whiteboardPath);

    m_buttonsList->clear();
    for (const auto &b : cfg.buttons) {
        auto *item = new QListWidgetItem(QString("%1 (%2)").arg(b.title, b.id));
        item->setData(Qt::UserRole, b.id);
        item->setData(Qt::UserRole + 1, b.title);
        item->setData(Qt::UserRole + 2, b.iconPath);
        item->setData(Qt::UserRole + 3, b.target);
        item->setData(Qt::UserRole + 4, b.actionType);
        item->setData(Qt::UserRole + 5, b.removable);
        m_buttonsList->addItem(item);
    }
}

void SettingsDialog::syncButtonsToConfig() {
    QVector<ButtonAction> buttons;
    for (int i = 0; i < m_buttonsList->count(); ++i) {
        auto *item = m_buttonsList->item(i);
        buttons.push_back(ButtonAction{
            item->data(Qt::UserRole).toString(),
            item->data(Qt::UserRole + 1).toString(),
            item->data(Qt::UserRole + 2).toString(),
            item->data(Qt::UserRole + 3).toString(),
            item->data(Qt::UserRole + 4).toString(),
            item->data(Qt::UserRole + 5).toBool()
        });
    }
    m_config->config().buttons = buttons;
}

void SettingsDialog::importClassList() {
    const QString file = QFileDialog::getOpenFileName(this, "导入班级名单", QString(), "名单 (*.csv *.txt *.xls *.xlsx)");
    if (file.isEmpty()) return;

    QFile f(file);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "失败", "文件读取失败。");
        return;
    }

    QVector<QString> names;
    QTextStream in(&f);
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        const auto cols = line.split(QRegExp("[,，;；\\t ]"), Qt::SkipEmptyParts);
        for (const QString &s : cols) names.push_back(s.trimmed());
    }

    m_config->config().attendance.allStudents = names;
    QMessageBox::information(this, "成功", QString("已导入 %1 名学生。\n(Excel 建议先另存为 CSV 以确保兼容性)").arg(names.size()));
}

void SettingsDialog::addCustomButton() {
    CustomButtonDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) return;
    const ButtonAction action = dialog.action();

    auto *item = new QListWidgetItem(QString("%1 (%2)").arg(action.title, action.id));
    item->setData(Qt::UserRole, action.id);
    item->setData(Qt::UserRole + 1, action.title);
    item->setData(Qt::UserRole + 2, action.iconPath);
    item->setData(Qt::UserRole + 3, action.target);
    item->setData(Qt::UserRole + 4, action.actionType);
    item->setData(Qt::UserRole + 5, action.removable);
    m_buttonsList->addItem(item);
}

void SettingsDialog::removeSelectedButton() {
    auto *item = m_buttonsList->currentItem();
    if (!item) return;
    if (!item->data(Qt::UserRole + 5).toBool()) {
        QMessageBox::information(this, "限制", "默认按钮不可删除。");
        return;
    }
    delete item;
}

void SettingsDialog::moveUp() {
    const int row = m_buttonsList->currentRow();
    if (row <= 0) return;
    auto *item = m_buttonsList->takeItem(row);
    m_buttonsList->insertItem(row - 1, item);
    m_buttonsList->setCurrentRow(row - 1);
}

void SettingsDialog::moveDown() {
    const int row = m_buttonsList->currentRow();
    if (row < 0 || row >= m_buttonsList->count() - 1) return;
    auto *item = m_buttonsList->takeItem(row);
    m_buttonsList->insertItem(row + 1, item);
    m_buttonsList->setCurrentRow(row + 1);
}

void SettingsDialog::chooseWhiteboardPath() {
    const QString file = QFileDialog::getOpenFileName(this, "选择希沃白板路径", QString(), "可执行文件或快捷方式 (*.exe *.lnk *.desktop);;所有文件 (*.*)");
    if (!file.isEmpty()) m_whiteboardPath->setText(file);
}

void SettingsDialog::saveAndClose() {
    auto &cfg = m_config->config();
    cfg.theme.darkMode = m_themeCombo->currentIndex() == 0;
    cfg.theme.sidebarWidth = m_sidebarWidth->value();
    cfg.theme.iconSize = m_iconSize->value();
    cfg.whiteboardPath = m_whiteboardPath->text().trimmed();
    syncButtonsToConfig();
    accept();
}
