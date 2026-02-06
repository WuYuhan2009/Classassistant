#include "custombuttondialog.h"

#include <QComboBox>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

CustomButtonDialog::CustomButtonDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("添加自定义按钮");
    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout;

    m_name = new QLineEdit;
    m_target = new QLineEdit;
    m_icon = new QLineEdit("assets/icons/custom.png");
    m_type = new QComboBox;
    m_type->addItems({"executable", "file", "url"});

    auto *targetBrowse = new QPushButton("浏览目标");
    auto *iconBrowse = new QPushButton("浏览图标");
    connect(targetBrowse, &QPushButton::clicked, this, &CustomButtonDialog::browseTarget);
    connect(iconBrowse, &QPushButton::clicked, this, &CustomButtonDialog::browseIcon);

    form->addRow("按钮名称", m_name);
    form->addRow("类型", m_type);
    form->addRow("目标", m_target);
    form->addRow("", targetBrowse);
    form->addRow("图标路径", m_icon);
    form->addRow("", iconBrowse);
    layout->addLayout(form);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

void CustomButtonDialog::browseTarget() {
    if (m_type->currentText() == "url") return;
    const QString path = QFileDialog::getOpenFileName(this, "选择目标");
    if (!path.isEmpty()) m_target->setText(path);
}

void CustomButtonDialog::browseIcon() {
    const QString path = QFileDialog::getOpenFileName(this, "选择图标", QString(), "图标 (*.png *.svg *.ico)");
    if (!path.isEmpty()) m_icon->setText(path);
}

ButtonAction CustomButtonDialog::action() const {
    return {
        QString("custom_%1").arg(QDateTime::currentMSecsSinceEpoch()),
        m_name->text().trimmed(),
        m_icon->text().trimmed(),
        m_target->text().trimmed(),
        m_type->currentText(),
        true
    };
}
