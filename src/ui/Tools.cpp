#include "Tools.h"

#include "../Utils.h"

#include <QApplication>
#include <QCloseEvent>
#include <QFileDialog>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QScreen>
#include <QVBoxLayout>
#include <QRandomGenerator>

AttendanceWidget::AttendanceWidget(QWidget* parent) : QWidget(parent) {
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    resize(300, 400);

    const QRect screen = QApplication::primaryScreen()->availableGeometry();
    move(screen.width() - 320, screen.height() - 450);

    auto* layout = new QVBoxLayout(this);
    auto* container = new QWidget;
    container->setStyleSheet("background-color: rgba(255,255,255,0.9); border-radius: 10px; border: 1px solid #ddd;");
    auto* innerLayout = new QVBoxLayout(container);

    auto* title = new QLabel("📅 班级考勤 (点击名字标记缺勤)");
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

    m_summary = new QLabel("加载中...");
    innerLayout->addWidget(m_summary);

    layout->addWidget(container);
    resetDaily();
}

void AttendanceWidget::resetDaily() {
    const QStringList students = Config::instance().getStudentList();
    m_table->setRowCount(students.size());
    for (int i = 0; i < students.size(); ++i) {
        m_table->setItem(i, 0, new QTableWidgetItem(students[i]));
        auto* status = new QTableWidgetItem("出勤");
        status->setForeground(QBrush(Qt::green));
        m_table->setItem(i, 1, status);
    }
    updateSummary();
}

void AttendanceWidget::onCellClicked(int row, int col) {
    Q_UNUSED(col);
    auto* item = m_table->item(row, 1);
    if (item->text() == "出勤") {
        item->setText("缺勤");
        item->setForeground(QBrush(Qt::red));
    } else {
        item->setText("出勤");
        item->setForeground(QBrush(Qt::green));
    }
    updateSummary();
}

void AttendanceWidget::updateSummary() {
    const int total = m_table->rowCount();
    int absent = 0;
    for (int i = 0; i < total; ++i) {
        if (m_table->item(i, 1)->text() == "缺勤") {
            ++absent;
        }
    }
    m_summary->setText(QString("应到: %1  实到: %2  缺勤: %3").arg(total).arg(total - absent).arg(absent));
}

void AttendanceWidget::closeEvent(QCloseEvent* event) {
    hide();
    event->ignore();
}

RandomCallDialog::RandomCallDialog(QWidget* parent) : QDialog(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
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

    auto* closeBtn = new QPushButton("关闭");
    closeBtn->setStyleSheet("background: rgba(255,255,255,0.3); border: none; border-radius: 5px; color: white; padding: 5px;");
    connect(closeBtn, &QPushButton::clicked, this, &RandomCallDialog::accept);

    l->addWidget(m_nameLabel);
    l->addWidget(closeBtn, 0, Qt::AlignCenter);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, [this]() {
        if (m_list.isEmpty()) {
            return;
        }

        m_nameLabel->setText(m_list[QRandomGenerator::global()->bounded(m_list.size())]);
        ++m_count;
        if (m_count > 20) {
            m_timer->setInterval(100);
        }
        if (m_count > 30) {
            m_timer->setInterval(300);
        }
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
        return;
    }

    m_count = 0;
    m_nameLabel->setStyleSheet("font-size: 40px; font-weight: bold; color: white;");
    m_timer->start(50);
    show();
}

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("ClassAssistant 设置");
    resize(500, 400);
    auto* layout = new QVBoxLayout(this);

    auto* h1 = new QHBoxLayout;
    h1->addWidget(new QLabel("希沃路径:"));
    m_seewoPathEdit = new QLineEdit(Config::instance().seewoPath);
    h1->addWidget(m_seewoPathEdit);
    auto* btnBrowse = new QPushButton("...");
    connect(btnBrowse, &QPushButton::clicked, [this]() {
        const QString p = QFileDialog::getOpenFileName(this, "选择exe", "", "Executable (*.exe)");
        if (!p.isEmpty()) {
            m_seewoPathEdit->setText(p);
        }
    });
    h1->addWidget(btnBrowse);
    layout->addLayout(h1);

    auto* btnImport = new QPushButton("导入班级名单 (TXT/CSV)");
    connect(btnImport, &QPushButton::clicked, this, &SettingsDialog::importStudents);
    layout->addWidget(btnImport);

    auto* tip = new QLabel("提示: 导入名单后请重启考勤面板。添加自定义按钮功能请修改 config.json。");
    tip->setWordWrap(true);
    layout->addWidget(tip);

    layout->addStretch();

    auto* btnSave = new QPushButton("保存并应用");
    connect(btnSave, &QPushButton::clicked, this, &SettingsDialog::saveData);
    layout->addWidget(btnSave);
}

void SettingsDialog::importStudents() {
    const QString path = QFileDialog::getOpenFileName(this, "选择名单", "", "Text Files (*.txt *.csv)");
    if (!path.isEmpty()) {
        Config::instance().importStudentsFromText(path);
        QMessageBox::information(this, "成功", "名单已导入");
    }
}

void SettingsDialog::saveData() {
    Config::instance().seewoPath = m_seewoPathEdit->text();
    Config::instance().save();
    emit configChanged();
    accept();
}
