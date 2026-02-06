#include "welcomedialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

WelcomeDialog::WelcomeDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("欢迎使用 Classassistant");
    resize(420, 260);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("首次启动，请完成基础设置："));

    auto *form = new QFormLayout;
    m_themeCombo = new QComboBox;
    m_themeCombo->addItems({"深色模式", "浅色模式"});
    m_sidebarWidth = new QSpinBox;
    m_sidebarWidth->setRange(72, 180);
    m_sidebarWidth->setValue(84);
    m_iconSize = new QSpinBox;
    m_iconSize->setRange(20, 64);
    m_iconSize->setValue(30);

    form->addRow("主题", m_themeCombo);
    form->addRow("侧栏宽度", m_sidebarWidth);
    form->addRow("图标大小", m_iconSize);

    layout->addLayout(form);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

bool WelcomeDialog::darkMode() const { return m_themeCombo->currentIndex() == 0; }
int WelcomeDialog::sidebarWidth() const { return m_sidebarWidth->value(); }
int WelcomeDialog::iconSize() const { return m_iconSize->value(); }
