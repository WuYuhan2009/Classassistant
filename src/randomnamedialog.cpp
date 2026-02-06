#include "randomnamedialog.h"
#include "ui_randomnamedialog.h"
#include "configmanager.h"

#include <QMessageBox>
#include <QRandomGenerator>
#include <QPropertyAnimation>
#include <QTimer>
#include <QEasingCurve>

RandomNameDialog::RandomNameDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::RandomNameDialog), m_animationTimer(new QTimer(this)), m_currentIndex(0), m_tickCount(0) {
    ui->setupUi(this);
    setupUI();
    loadStudents();
    connect(m_animationTimer, &QTimer::timeout, this, &RandomNameDialog::updateRandomName);
    connect(ui->startButton, &QPushButton::clicked, this, &RandomNameDialog::startRandom);
    connect(ui->closeButton, &QPushButton::clicked, this, &QDialog::accept);
}

RandomNameDialog::~RandomNameDialog() { delete ui; }

void RandomNameDialog::setupUI() { setFixedSize(400, 300); }

void RandomNameDialog::loadStudents() {
    m_students = ConfigManager::instance().getStudents();
    ui->nameLabel->setText(m_students.isEmpty() ? "未导入名单" : "点击开始");
    ui->startButton->setEnabled(!m_students.isEmpty());
}

void RandomNameDialog::startRandom() {
    if (m_students.isEmpty()) { QMessageBox::warning(this, "提示", "请先在设置中导入学生名单！"); return; }
    ui->startButton->setEnabled(false);
    m_tickCount = 0;
    m_animationTimer->start(50);
}

void RandomNameDialog::stopRandom() {
    m_animationTimer->stop();
    ui->startButton->setText("再来一次");
    ui->startButton->setEnabled(true);
    auto *animation = new QPropertyAnimation(ui->nameLabel, "geometry");
    animation->setDuration(300);
    animation->setStartValue(ui->nameLabel->geometry());
    QRect endRect = ui->nameLabel->geometry(); endRect.adjust(-10, -10, 10, 10);
    animation->setEndValue(endRect);
    animation->setEasingCurve(QEasingCurve::OutBounce);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void RandomNameDialog::updateRandomName() {
    m_tickCount++;
    m_currentIndex = QRandomGenerator::global()->bounded(m_students.size());
    ui->nameLabel->setText(m_students[m_currentIndex]);
    if (m_tickCount >= 30) stopRandom();
}
