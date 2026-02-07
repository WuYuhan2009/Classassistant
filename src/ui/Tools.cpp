#include "Tools.h"

#include <QApplication>
#include <QDate>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QRandomGenerator>
#include <QScreen>
#include <QVBoxLayout>

namespace {
QString buttonStyle() {
    return "QPushButton{background:#ffffff;border:1px solid #c8c8c8;border-radius:10px;font-weight:600;}"
           "QPushButton:hover{background:#f5f9ff;}";
}
}

AttendanceSummaryWidget::AttendanceSummaryWidget(QWidget* parent) : QWidget(parent) {
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    auto* panel = new QWidget;
    panel->setStyleSheet("background:#fff7d6;border:2px solid #f0c14b;border-radius:12px;");
    auto* inner = new QVBoxLayout(panel);

    m_title = new QLabel("📌 今日考勤概览");
    m_title->setStyleSheet("font-size:16px;font-weight:700;color:#7a4b00;");

    m_counts = new QLabel;
    m_counts->setStyleSheet("font-size:15px;font-weight:700;color:#8b1e1e;");

    m_absentList = new QLabel;
    m_absentList->setWordWrap(true);
    m_absentList->setStyleSheet("font-size:14px;font-weight:600;color:#8b1e1e;background:#fff3f3;border:1px solid #f3c5c5;border-radius:8px;padding:8px;");

    inner->addWidget(m_title);
    inner->addWidget(m_counts);
    inner->addWidget(m_absentList);
    root->addWidget(panel);

    resetDaily();
}

void AttendanceSummaryWidget::syncDaily() {
    const QString today = QDate::currentDate().toString(Qt::ISODate);
    if (m_lastResetDate != today) {
        m_lastResetDate = today;
        m_absentees.clear();
    }
}

void AttendanceSummaryWidget::resetDaily() {
    m_lastResetDate = QDate::currentDate().toString(Qt::ISODate);
    m_absentees.clear();
    refreshUi();
}

void AttendanceSummaryWidget::applyAbsentees(const QStringList& absentees) {
    syncDaily();
    m_absentees = absentees;
    refreshUi();
}

void AttendanceSummaryWidget::refreshUi() {
    const int total = Config::instance().getStudentList().size();
    const int absent = m_absentees.size();
    const int present = qMax(0, total - absent);

    m_counts->setText(QString("应到：%1   实到：%2").arg(total).arg(present));
    m_absentList->setText(QString("缺勤人员：%1").arg(m_absentees.isEmpty() ? "无" : m_absentees.join("、")));

    setFixedWidth(Config::instance().attendanceSummaryWidth);
    adjustSize();

    const QRect screen = QApplication::primaryScreen()->availableGeometry();
    const int x = screen.right() - width() - 12;
    const int y = screen.bottom() - height() - 12;
    move(x, y);
}

void AttendanceSummaryWidget::closeEvent(QCloseEvent* event) {
    hide();
    event->ignore();
}

AttendanceSelectDialog::AttendanceSelectDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("考勤选择（选择缺勤人员）");
    resize(420, 520);
    setWindowFlags(windowFlags() | Qt::Tool);

    auto* layout = new QVBoxLayout(this);
    auto* tip = new QLabel("请选择今日缺勤人员，点击保存后将同步到底部考勤概览。\n（名单窗口可关闭，考勤概览窗口不会关闭）");
    tip->setWordWrap(true);
    layout->addWidget(tip);

    m_roster = new QListWidget;
    m_roster->setSelectionMode(QAbstractItemView::MultiSelection);
    const auto students = Config::instance().getStudentList();
    for (const auto& s : students) {
        auto* item = new QListWidgetItem(s);
        item->setCheckState(Qt::Unchecked);
        m_roster->addItem(item);
    }
    layout->addWidget(m_roster, 1);

    auto* actions = new QHBoxLayout;
    auto* saveBtn = new QPushButton("保存缺勤名单");
    saveBtn->setStyleSheet(buttonStyle());
    auto* cancelBtn = new QPushButton("关闭");
    cancelBtn->setStyleSheet(buttonStyle());
    connect(saveBtn, &QPushButton::clicked, this, &AttendanceSelectDialog::saveSelection);
    connect(cancelBtn, &QPushButton::clicked, this, &AttendanceSelectDialog::hide);
    actions->addWidget(saveBtn);
    actions->addWidget(cancelBtn);
    layout->addLayout(actions);
}

void AttendanceSelectDialog::setSelectedAbsentees(const QStringList& absentees) {
    for (int i = 0; i < m_roster->count(); ++i) {
        auto* item = m_roster->item(i);
        item->setCheckState(absentees.contains(item->text()) ? Qt::Checked : Qt::Unchecked);
    }
}

void AttendanceSelectDialog::saveSelection() {
    QStringList absentees;
    for (int i = 0; i < m_roster->count(); ++i) {
        auto* item = m_roster->item(i);
        if (item->checkState() == Qt::Checked) {
            absentees.append(item->text());
        }
    }
    emit saved(absentees);
    hide();
}

void AttendanceSelectDialog::closeEvent(QCloseEvent* event) {
    hide();
    event->ignore();
}

RandomCallDialog::RandomCallDialog(QWidget* parent) : QDialog(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    resize(400, 200);

    auto* layout = new QVBoxLayout(this);
    auto* bg = new QLabel(this);
    bg->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 #85C1E9, stop:1 #2E86C1); border-radius: 20px; color: white;");
    layout->addWidget(bg);

    auto* l = new QVBoxLayout(bg);
    m_nameLabel = new QLabel("准备点名...");
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setStyleSheet("font-size: 40px; font-weight: bold;");
    l->addWidget(m_nameLabel);

    auto* closeBtn = new QPushButton("隐藏");
    closeBtn->setStyleSheet(buttonStyle());
    connect(closeBtn, &QPushButton::clicked, this, &RandomCallDialog::hide);
    l->addWidget(closeBtn, 0, Qt::AlignCenter);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, [this]() {
        if (m_list.isEmpty()) {
            return;
        }
        m_nameLabel->setText(m_list[QRandomGenerator::global()->bounded(m_list.size())]);
        ++m_count;
        if (m_count > 20) m_timer->setInterval(100);
        if (m_count > 30) m_timer->setInterval(300);
        if (m_count > 35) {
            m_timer->stop();
            m_nameLabel->setStyleSheet("font-size: 50px; font-weight: bold; color: #F1C40F;");
        }
    });
}

void RandomCallDialog::startAnim() {
    m_list = Config::instance().getStudentList();
    if (m_list.isEmpty()) {
        m_nameLabel->setText("无名单");
        show();
        return;
    }
    m_count = 0;
    m_nameLabel->setStyleSheet("font-size: 40px; font-weight: bold; color: white;");
    m_timer->start(50);
    show();
}

void RandomCallDialog::closeEvent(QCloseEvent* event) {
    hide();
    event->ignore();
}

AddButtonDialog::AddButtonDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("添加自定义按钮");
    resize(420, 260);

    auto* layout = new QVBoxLayout(this);

    m_nameEdit = new QLineEdit;
    m_nameEdit->setPlaceholderText("按钮名称");
    layout->addWidget(new QLabel("按钮名称"));
    layout->addWidget(m_nameEdit);

    m_iconEdit = new QLineEdit;
    auto* iconBtn = new QPushButton("选择图标");
    iconBtn->setStyleSheet(buttonStyle());
    connect(iconBtn, &QPushButton::clicked, [this]() {
        const QString p = QFileDialog::getOpenFileName(this, "选择图标", "", "Images (*.png *.jpg *.ico *.svg)");
        if (!p.isEmpty()) m_iconEdit->setText(p);
    });

    auto* iconLayout = new QHBoxLayout;
    iconLayout->addWidget(m_iconEdit);
    iconLayout->addWidget(iconBtn);
    layout->addWidget(new QLabel("图标路径"));
    layout->addLayout(iconLayout);

    m_actionCombo = new QComboBox;
    m_actionCombo->addItem("打开程序/文件", "exe");
    m_actionCombo->addItem("打开链接(URL)", "url");
    m_actionCombo->addItem("内置功能(func)", "func");
    layout->addWidget(new QLabel("动作类型"));
    layout->addWidget(m_actionCombo);

    m_targetEdit = new QLineEdit;
    m_targetEdit->setPlaceholderText("路径 / URL / 功能标识");
    layout->addWidget(new QLabel("目标"));
    layout->addWidget(m_targetEdit);

    auto* actions = new QHBoxLayout;
    auto* ok = new QPushButton("确定");
    auto* cancel = new QPushButton("取消");
    ok->setStyleSheet(buttonStyle());
    cancel->setStyleSheet(buttonStyle());
    connect(ok, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    actions->addStretch();
    actions->addWidget(ok);
    actions->addWidget(cancel);
    layout->addLayout(actions);
}

AppButton AddButtonDialog::resultButton() const {
    return {m_nameEdit->text().trimmed(),
            m_iconEdit->text().trimmed(),
            m_actionCombo->currentData().toString(),
            m_targetEdit->text().trimmed(),
            false};
}

FirstRunWizard::FirstRunWizard(QWidget* parent) : QDialog(parent) {
    setWindowTitle("欢迎使用 ClassAssistant");
    resize(520, 420);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    auto* layout = new QVBoxLayout(this);
    auto* intro = new QLabel("首次启动向导：请完成基础初始化设置（后续可在设置中修改）");
    intro->setWordWrap(true);
    layout->addWidget(intro);

    m_darkMode = new QCheckBox("启用深色模式");
    layout->addWidget(m_darkMode);

    layout->addWidget(new QLabel("功能按钮图标大小"));
    m_iconSize = new QSlider(Qt::Horizontal);
    m_iconSize->setRange(28, 72);
    m_iconSize->setValue(Config::instance().iconSize);
    layout->addWidget(m_iconSize);

    layout->addWidget(new QLabel("展开球不透明度"));
    m_floatingOpacity = new QSlider(Qt::Horizontal);
    m_floatingOpacity->setRange(35, 100);
    m_floatingOpacity->setValue(Config::instance().floatingOpacity);
    layout->addWidget(m_floatingOpacity);

    layout->addWidget(new QLabel("考勤概览宽度"));
    m_summaryWidth = new QSlider(Qt::Horizontal);
    m_summaryWidth->setRange(300, 520);
    m_summaryWidth->setValue(Config::instance().attendanceSummaryWidth);
    layout->addWidget(m_summaryWidth);

    m_startCollapsed = new QCheckBox("启动后默认收起到右下角悬浮球");
    m_startCollapsed->setChecked(Config::instance().startCollapsed);
    layout->addWidget(m_startCollapsed);

    layout->addWidget(new QLabel("默认程序路径（希沃）"));
    m_seewoPathEdit = new QLineEdit(Config::instance().seewoPath);
    auto* browse = new QPushButton("选择程序路径");
    browse->setStyleSheet(buttonStyle());
    connect(browse, &QPushButton::clicked, [this]() {
        const QString p = QFileDialog::getOpenFileName(this, "选择程序", "", "Executable (*.exe);;All Files (*)");
        if (!p.isEmpty()) m_seewoPathEdit->setText(p);
    });
    layout->addWidget(m_seewoPathEdit);
    layout->addWidget(browse);

    auto* done = new QPushButton("完成初始化");
    done->setStyleSheet(buttonStyle());
    connect(done, &QPushButton::clicked, this, &FirstRunWizard::finishSetup);
    layout->addStretch();
    layout->addWidget(done);
}

void FirstRunWizard::finishSetup() {
    auto& cfg = Config::instance();
    cfg.darkMode = m_darkMode->isChecked();
    cfg.iconSize = m_iconSize->value();
    cfg.floatingOpacity = m_floatingOpacity->value();
    cfg.attendanceSummaryWidth = m_summaryWidth->value();
    cfg.startCollapsed = m_startCollapsed->isChecked();
    cfg.seewoPath = m_seewoPathEdit->text().trimmed();
    cfg.firstRunCompleted = true;
    cfg.save();
    accept();
}

void FirstRunWizard::closeEvent(QCloseEvent* event) {
    event->ignore();
}

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("ClassAssistant 设置");
    resize(640, 580);

    auto* layout = new QVBoxLayout(this);

    m_darkMode = new QCheckBox("深色模式");
    layout->addWidget(m_darkMode);

    layout->addWidget(new QLabel("按钮图标大小"));
    m_iconSize = new QSlider(Qt::Horizontal);
    m_iconSize->setRange(28, 72);
    layout->addWidget(m_iconSize);

    layout->addWidget(new QLabel("悬浮球透明度"));
    m_floatingOpacity = new QSlider(Qt::Horizontal);
    m_floatingOpacity->setRange(35, 100);
    layout->addWidget(m_floatingOpacity);

    layout->addWidget(new QLabel("考勤概览宽度"));
    m_summaryWidth = new QSlider(Qt::Horizontal);
    m_summaryWidth->setRange(300, 520);
    layout->addWidget(m_summaryWidth);

    m_startCollapsed = new QCheckBox("启动时收起到悬浮球");
    layout->addWidget(m_startCollapsed);

    auto* pathLayout = new QHBoxLayout;
    m_seewoPathEdit = new QLineEdit;
    auto* choosePath = new QPushButton("选择路径");
    choosePath->setStyleSheet(buttonStyle());
    connect(choosePath, &QPushButton::clicked, [this]() {
        const QString p = QFileDialog::getOpenFileName(this, "选择可执行文件", "", "Executable (*.exe);;All Files (*)");
        if (!p.isEmpty()) m_seewoPathEdit->setText(p);
    });
    pathLayout->addWidget(m_seewoPathEdit);
    pathLayout->addWidget(choosePath);
    layout->addWidget(new QLabel("默认程序路径（希沃）"));
    layout->addLayout(pathLayout);

    auto* importBtn = new QPushButton("导入班级名单（Excel/CSV/TXT）");
    importBtn->setStyleSheet(buttonStyle());
    connect(importBtn, &QPushButton::clicked, this, &SettingsDialog::importStudents);
    layout->addWidget(importBtn);

    layout->addWidget(new QLabel("按钮管理（默认系统按钮不可删除）"));
    m_buttonList = new QListWidget;
    layout->addWidget(m_buttonList, 1);

    auto* btnOps = new QHBoxLayout;
    auto* btnAdd = new QPushButton("添加按钮");
    auto* btnRemove = new QPushButton("删除按钮");
    auto* btnUp = new QPushButton("上移");
    auto* btnDown = new QPushButton("下移");
    btnAdd->setStyleSheet(buttonStyle());
    btnRemove->setStyleSheet(buttonStyle());
    btnUp->setStyleSheet(buttonStyle());
    btnDown->setStyleSheet(buttonStyle());
    connect(btnAdd, &QPushButton::clicked, this, &SettingsDialog::addButton);
    connect(btnRemove, &QPushButton::clicked, this, &SettingsDialog::removeButton);
    connect(btnUp, &QPushButton::clicked, this, &SettingsDialog::moveUp);
    connect(btnDown, &QPushButton::clicked, this, &SettingsDialog::moveDown);
    btnOps->addWidget(btnAdd);
    btnOps->addWidget(btnRemove);
    btnOps->addWidget(btnUp);
    btnOps->addWidget(btnDown);
    layout->addLayout(btnOps);

    auto* save = new QPushButton("保存并应用");
    save->setStyleSheet(buttonStyle());
    connect(save, &QPushButton::clicked, this, &SettingsDialog::saveData);
    layout->addWidget(save);

    loadData();
}

void SettingsDialog::loadData() {
    const auto& cfg = Config::instance();
    m_darkMode->setChecked(cfg.darkMode);
    m_iconSize->setValue(cfg.iconSize);
    m_floatingOpacity->setValue(cfg.floatingOpacity);
    m_summaryWidth->setValue(cfg.attendanceSummaryWidth);
    m_startCollapsed->setChecked(cfg.startCollapsed);
    m_seewoPathEdit->setText(cfg.seewoPath);

    m_buttonList->clear();
    const auto buttons = cfg.getButtons();
    for (const auto& b : buttons) {
        auto* item = new QListWidgetItem(QString("%1 [%2]").arg(b.name, b.action));
        item->setData(Qt::UserRole, b.name);
        item->setData(Qt::UserRole + 1, b.iconPath);
        item->setData(Qt::UserRole + 2, b.action);
        item->setData(Qt::UserRole + 3, b.target);
        item->setData(Qt::UserRole + 4, b.isSystem);
        m_buttonList->addItem(item);
    }
}

void SettingsDialog::importStudents() {
    const QString path = QFileDialog::getOpenFileName(this, "选择名单", "", "Roster Files (*.xlsx *.xls *.csv *.txt)");
    if (path.isEmpty()) {
        return;
    }

    QString error;
    if (!Config::instance().importStudentsFromText(path, &error)) {
        QMessageBox::warning(this, "导入失败", error);
        return;
    }
    QMessageBox::information(this, "成功", "名单导入成功。");
}

void SettingsDialog::addButton() {
    AddButtonDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const AppButton button = dialog.resultButton();
    if (button.name.isEmpty() || button.action.isEmpty() || button.target.isEmpty()) {
        QMessageBox::warning(this, "提示", "名称、动作、目标不能为空。");
        return;
    }

    auto* item = new QListWidgetItem(QString("%1 [%2]").arg(button.name, button.action));
    item->setData(Qt::UserRole, button.name);
    item->setData(Qt::UserRole + 1, button.iconPath);
    item->setData(Qt::UserRole + 2, button.action);
    item->setData(Qt::UserRole + 3, button.target);
    item->setData(Qt::UserRole + 4, false);
    m_buttonList->addItem(item);
}

void SettingsDialog::removeButton() {
    auto* item = m_buttonList->currentItem();
    if (!item) return;

    if (item->data(Qt::UserRole + 4).toBool()) {
        QMessageBox::warning(this, "提示", "默认系统按钮不可删除。");
        return;
    }
    delete item;
}

void SettingsDialog::moveUp() {
    const int row = m_buttonList->currentRow();
    if (row <= 0) return;
    auto* item = m_buttonList->takeItem(row);
    m_buttonList->insertItem(row - 1, item);
    m_buttonList->setCurrentRow(row - 1);
}

void SettingsDialog::moveDown() {
    const int row = m_buttonList->currentRow();
    if (row < 0 || row >= m_buttonList->count() - 1) return;
    auto* item = m_buttonList->takeItem(row);
    m_buttonList->insertItem(row + 1, item);
    m_buttonList->setCurrentRow(row + 1);
}

void SettingsDialog::saveData() {
    auto& cfg = Config::instance();
    cfg.darkMode = m_darkMode->isChecked();
    cfg.iconSize = m_iconSize->value();
    cfg.floatingOpacity = m_floatingOpacity->value();
    cfg.attendanceSummaryWidth = m_summaryWidth->value();
    cfg.startCollapsed = m_startCollapsed->isChecked();
    cfg.seewoPath = m_seewoPathEdit->text().trimmed();

    QVector<AppButton> buttons;
    for (int i = 0; i < m_buttonList->count(); ++i) {
        auto* item = m_buttonList->item(i);
        buttons.append({item->data(Qt::UserRole).toString(),
                        item->data(Qt::UserRole + 1).toString(),
                        item->data(Qt::UserRole + 2).toString(),
                        item->data(Qt::UserRole + 3).toString(),
                        item->data(Qt::UserRole + 4).toBool()});
    }

    cfg.setButtons(buttons);
    cfg.save();
    emit configChanged();
    hide();
}

void SettingsDialog::closeEvent(QCloseEvent* event) {
    hide();
    event->ignore();
}
