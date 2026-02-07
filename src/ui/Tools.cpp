#include "Tools.h"

#include <QApplication>
#include <QDate>
#include <QFileDialog>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QRandomGenerator>
#include <QScreen>
#include <QVBoxLayout>

AttendanceWidget::AttendanceWidget(QWidget* parent) : QWidget(parent) {
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    resize(360, 280);

    const QRect screen = QApplication::primaryScreen()->availableGeometry();
    move(screen.width() - 380, screen.height() - 320);

    auto* layout = new QVBoxLayout(this);
    auto* container = new QWidget;
    container->setStyleSheet("background-color: rgba(255,255,255,0.92); border-radius: 10px; border: 1px solid #ddd;");
    auto* innerLayout = new QVBoxLayout(container);

    auto* title = new QLabel("📅 今日考勤（仅选择未出勤/请假）");
    title->setStyleSheet("font-weight: bold; font-size: 14px; color: #333;");
    innerLayout->addWidget(title);

    m_table = new QTableWidget;
    m_table->setColumnCount(2);
    m_table->setHorizontalHeaderLabels({"姓名", "状态"});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionMode(QAbstractItemView::NoSelection);
    connect(m_table, &QTableWidget::cellClicked, this, &AttendanceWidget::onCellClicked);
    innerLayout->addWidget(m_table);

    m_summary = new QLabel;
    innerLayout->addWidget(m_summary);

    layout->addWidget(container);
    resetDaily();
}

void AttendanceWidget::checkDailyReset() {
    const QString today = QDate::currentDate().toString(Qt::ISODate);
    if (today != m_lastResetDate) {
        m_lastResetDate = today;
        resetDaily();
    }
}

void AttendanceWidget::resetDaily() {
    m_lastResetDate = QDate::currentDate().toString(Qt::ISODate);
    const QStringList students = Config::instance().getStudentList();
    m_table->setRowCount(students.size());

    for (int i = 0; i < students.size(); ++i) {
        m_table->setItem(i, 0, new QTableWidgetItem(students[i]));
        auto* status = new QTableWidgetItem("出勤");
        status->setForeground(QBrush(Qt::darkGreen));
        m_table->setItem(i, 1, status);
    }
    updateSummary();
}

void AttendanceWidget::onCellClicked(int row, int col) {
    Q_UNUSED(col);
    checkDailyReset();
    auto* item = m_table->item(row, 1);
    if (item->text() == "出勤") {
        item->setText("请假");
        item->setForeground(QBrush(Qt::red));
    } else {
        item->setText("出勤");
        item->setForeground(QBrush(Qt::darkGreen));
    }
    updateSummary();
}

void AttendanceWidget::updateSummary() {
    const int total = m_table->rowCount();
    int leaveCount = 0;
    QStringList leaveNames;

    for (int i = 0; i < total; ++i) {
        auto* state = m_table->item(i, 1);
        if (state && state->text() == "请假") {
            ++leaveCount;
            leaveNames.append(m_table->item(i, 0)->text());
        }
    }

    const int present = total - leaveCount;
    m_summary->setText(QString("应到: %1  实到: %2\n请假: %3")
                           .arg(total)
                           .arg(present)
                           .arg(leaveNames.isEmpty() ? "无" : leaveNames.join("、")));
}

void AttendanceWidget::closeEvent(QCloseEvent* event) {
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
    closeBtn->setStyleSheet("background: rgba(255,255,255,0.3); border: none; border-radius: 5px; color: white; padding: 5px;");
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
    resize(480, 340);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("首次启动向导：请完成基础配置"));

    m_darkMode = new QCheckBox("启用深色模式");
    layout->addWidget(m_darkMode);

    layout->addWidget(new QLabel("侧栏宽度"));
    m_sidebarWidth = new QSlider(Qt::Horizontal);
    m_sidebarWidth->setRange(60, 180);
    m_sidebarWidth->setValue(Config::instance().sidebarWidth);
    layout->addWidget(m_sidebarWidth);

    layout->addWidget(new QLabel("图标大小"));
    m_iconSize = new QSlider(Qt::Horizontal);
    m_iconSize->setRange(24, 64);
    m_iconSize->setValue(Config::instance().iconSize);
    layout->addWidget(m_iconSize);

    layout->addWidget(new QLabel("希沃路径"));
    m_seewoPathEdit = new QLineEdit(Config::instance().seewoPath);
    auto* browse = new QPushButton("选择程序路径");
    connect(browse, &QPushButton::clicked, [this]() {
        const QString p = QFileDialog::getOpenFileName(this, "选择程序", "", "Executable (*.exe);;All Files (*)");
        if (!p.isEmpty()) m_seewoPathEdit->setText(p);
    });
    layout->addWidget(m_seewoPathEdit);
    layout->addWidget(browse);

    auto* done = new QPushButton("完成");
    connect(done, &QPushButton::clicked, this, &FirstRunWizard::finishSetup);
    layout->addStretch();
    layout->addWidget(done);
}

void FirstRunWizard::finishSetup() {
    auto& cfg = Config::instance();
    cfg.darkMode = m_darkMode->isChecked();
    cfg.sidebarWidth = m_sidebarWidth->value();
    cfg.iconSize = m_iconSize->value();
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
    resize(620, 520);

    auto* layout = new QVBoxLayout(this);

    m_darkMode = new QCheckBox("深色模式");
    layout->addWidget(m_darkMode);

    layout->addWidget(new QLabel("侧栏宽度"));
    m_sidebarWidth = new QSlider(Qt::Horizontal);
    m_sidebarWidth->setRange(60, 180);
    layout->addWidget(m_sidebarWidth);

    layout->addWidget(new QLabel("图标大小"));
    m_iconSize = new QSlider(Qt::Horizontal);
    m_iconSize->setRange(24, 64);
    layout->addWidget(m_iconSize);

    auto* pathLayout = new QHBoxLayout;
    m_seewoPathEdit = new QLineEdit;
    auto* choosePath = new QPushButton("选择路径");
    connect(choosePath, &QPushButton::clicked, [this]() {
        const QString p = QFileDialog::getOpenFileName(this, "选择可执行文件", "", "Executable (*.exe);;All Files (*)");
        if (!p.isEmpty()) m_seewoPathEdit->setText(p);
    });
    pathLayout->addWidget(m_seewoPathEdit);
    pathLayout->addWidget(choosePath);
    layout->addWidget(new QLabel("默认程序路径（希沃）"));
    layout->addLayout(pathLayout);

    auto* importBtn = new QPushButton("导入班级名单（Excel/CSV/TXT）");
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
    connect(save, &QPushButton::clicked, this, &SettingsDialog::saveData);
    layout->addWidget(save);

    loadData();
}

void SettingsDialog::loadData() {
    const auto& cfg = Config::instance();
    m_darkMode->setChecked(cfg.darkMode);
    m_sidebarWidth->setValue(cfg.sidebarWidth);
    m_iconSize->setValue(cfg.iconSize);
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
        if (b.isSystem) {
            item->setToolTip("系统按钮，禁止删除");
        }
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
    cfg.sidebarWidth = m_sidebarWidth->value();
    cfg.iconSize = m_iconSize->value();
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
