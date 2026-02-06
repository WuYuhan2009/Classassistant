#include "attendancewidget.h"
#include "configmanager.h"

#include <QVBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QTimer>
#include <QDateTime>
#include <QLabel>
#include <QListWidget>

AttendanceWidget::AttendanceWidget(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool) {
    setAttribute(Qt::WA_TranslucentBackground);
    setupUI();
    loadStudents();
    updateDisplay();
    positionWindow();
    startDailyTimer();
}

void AttendanceWidget::setupUI() {
    setFixedSize(300, 400);
    auto *mainLayout = new QVBoxLayout(this);

    QLabel *titleLabel = new QLabel("班级考勤", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    m_totalLabel = new QLabel(this);
    m_presentLabel = new QLabel(this);
    m_absentLabel = new QLabel(this);
    m_absentLabel->setWordWrap(true);
    mainLayout->addWidget(m_totalLabel);
    mainLayout->addWidget(m_presentLabel);
    mainLayout->addWidget(m_absentLabel);

    mainLayout->addWidget(new QLabel("点击学生标记缺勤:", this));
    m_studentList = new QListWidget(this);
    m_studentList->setSelectionMode(QAbstractItemView::NoSelection);
    connect(m_studentList, &QListWidget::itemClicked, this, &AttendanceWidget::onStudentClicked);
    mainLayout->addWidget(m_studentList);
}

void AttendanceWidget::loadStudents() {
    m_allStudents = ConfigManager::instance().getStudents();
    m_absentStudents.clear();
    m_studentList->clear();
    for (const QString &s : m_allStudents) {
        auto *item = new QListWidgetItem(s);
        item->setCheckState(Qt::Unchecked);
        m_studentList->addItem(item);
    }
}

void AttendanceWidget::updateDisplay() {
    int total = m_allStudents.size();
    int present = total - m_absentStudents.size();
    m_totalLabel->setText(QString("应到人数: %1").arg(total));
    m_presentLabel->setText(QString("实到人数: %1").arg(present));
    m_absentLabel->setText(m_absentStudents.isEmpty() ? "请假名单: 无" : QString("请假名单: %1").arg(m_absentStudents.join("、")));
}

void AttendanceWidget::onStudentClicked(QListWidgetItem *item) {
    QString studentName = item->text();
    if (item->checkState() == Qt::Unchecked) {
        item->setCheckState(Qt::Checked);
        if (!m_absentStudents.contains(studentName)) m_absentStudents.append(studentName);
    } else {
        item->setCheckState(Qt::Unchecked);
        m_absentStudents.removeOne(studentName);
    }
    updateDisplay();
}

void AttendanceWidget::positionWindow() {
    QRect r = QApplication::primaryScreen()->availableGeometry();
    move(r.right() - width() - 20, r.bottom() - height() - 20);
}

void AttendanceWidget::startDailyTimer() {
    auto *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &AttendanceWidget::resetDaily);
    auto now = QDateTime::currentDateTime();
    auto midnight = now.addDays(1);
    midnight.setTime(QTime(0,0,0));
    QTimer::singleShot(now.msecsTo(midnight), this, [this, timer]() {
        resetDaily();
        timer->start(24 * 60 * 60 * 1000);
    });
}

void AttendanceWidget::resetDaily() {
    m_absentStudents.clear();
    for (int i = 0; i < m_studentList->count(); ++i) {
        m_studentList->item(i)->setCheckState(Qt::Unchecked);
    }
    updateDisplay();
}
