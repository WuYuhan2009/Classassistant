#include "Tools.h"

#include <QApplication>
#include <QClipboard>
#include <QCoreApplication>
#include <QDate>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QGuiApplication>
#include <QMessageBox>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QRandomGenerator>
#include <QSet>
#include <QScreen>
#include <QTextStream>
#include <QTime>
#include <QVBoxLayout>

namespace {
QString buttonStylePrimary() {
    return "QPushButton{background:#ffffff;border:1px solid #d8e0eb;border-radius:10px;font-weight:600;padding:8px 12px;color:#1f2d3d;}"
           "QPushButton:hover{background:#f4f8fd;}";
}

QString cardStyle() {
    return "background:#ffffff;border:1px solid #dfe5ee;border-radius:14px;";
}

void decorateDialog(QDialog* dlg, const QString& title) {
    dlg->setWindowTitle(title);
    dlg->setWindowFlags((dlg->windowFlags() | Qt::Tool) & ~Qt::WindowContextHelpButtonHint);
    dlg->setStyleSheet("QDialog{background:#f5f8fc;} QLabel{color:#223042;} "
                       "QLineEdit,QTextEdit,QListWidget,QTreeWidget,QComboBox,QSpinBox,QTableWidget{"
                       "background:#ffffff;border:1px solid #d8e0eb;border-radius:10px;padding:6px;}"
                       "QTreeWidget::item{height:28px;border-radius:8px;}"
                       "QTreeWidget::item:selected{background:#e9f2ff;color:#1f4f8f;}"
                       "QCheckBox{spacing:8px;} "
                       "QSlider::groove:horizontal{height:6px;background:#dbe4ef;border-radius:3px;}"
                       "QSlider::handle:horizontal{width:16px;margin:-5px 0;background:#ffffff;border:1px solid #9cb2ce;border-radius:8px;}"
                       "QGroupBox{font-weight:700;border:1px solid #dfe5ee;border-radius:12px;margin-top:10px;padding-top:12px;background:#ffffff;}"
                       "QGroupBox::title{subcontrol-origin:margin;left:10px;padding:0 6px;}"
                       "QScrollBar:vertical{background:transparent;width:10px;margin:2px;}"
                       "QScrollBar::handle:vertical{background:#c8d8ec;min-height:20px;border-radius:5px;}"
                       "QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical{height:0;}"
                       "QPushButton{border-radius:12px;}");
}
}

void smoothShow(QWidget* w) {
    if (!w) return;
    w->setWindowOpacity(0.0);
    w->show();
    w->raise();
    w->activateWindow();
    auto* anim = new QPropertyAnimation(w, "windowOpacity", w);
    anim->setDuration(Config::instance().animationDurationMs);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutBack);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void smoothHide(QWidget* w) {
    if (!w || !w->isVisible()) return;
    auto* anim = new QPropertyAnimation(w, "windowOpacity", w);
    anim->setDuration(Config::instance().animationDurationMs);
    anim->setStartValue(w->windowOpacity());
    anim->setEndValue(0.0);
    anim->setEasingCurve(QEasingCurve::InOutCubic);
    QObject::connect(anim, &QPropertyAnimation::finished, w, [w]() {
        w->hide();
        w->setWindowOpacity(1.0);
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

AttendanceSummaryWidget::AttendanceSummaryWidget(QWidget* parent) : QWidget(parent) {
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    auto* panel = new QWidget;
    panel->setStyleSheet("background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #ffffff,stop:1 #f5f9ff);border:1px solid #d9e4f2;border-radius:18px;");
    auto* inner = new QVBoxLayout(panel);
    inner->setSpacing(10);

    m_title = new QLabel("今日考勤概览");
    m_title->setStyleSheet("font-size:18px;font-weight:800;color:#2a4362;");

    m_counts = new QLabel;
    m_counts->setStyleSheet("font-size:16px;font-weight:800;color:#2f5e90;");

    m_absentList = new QLabel;
    m_absentList->setWordWrap(true);
    m_absentList->setStyleSheet("font-size:14px;background:#ffffff;border:1px solid #dbe6f3;border-radius:12px;padding:10px;");

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

    m_counts->setText(QString("应到：%1   实到：%2   缺勤：%3").arg(total).arg(present).arg(absent));
    m_title->setText(QString("今日考勤概览  ·  %1").arg(QTime::currentTime().toString("HH:mm")));
    m_absentList->setText(QString("缺勤人员：%1").arg(m_absentees.isEmpty() ? "无" : m_absentees.join("、")));

    setFixedWidth(Config::instance().attendanceSummaryWidth);
    adjustSize();

    const QRect screen = QApplication::primaryScreen()->availableGeometry();
    move(screen.right() - width() - 12, screen.top() + 12);
}

void AttendanceSummaryWidget::closeEvent(QCloseEvent* event) {
    smoothHide(this);
    event->ignore();
}

AttendanceSelectDialog::AttendanceSelectDialog(QWidget* parent) : QDialog(parent) {
    decorateDialog(this, "考勤选择（勾选缺勤学生）");
    setFixedSize(560, 560);

    auto* layout = new QVBoxLayout(this);
    auto* tip = new QLabel("请选择今日缺勤人员。支持搜索、导出与一键全员到齐。");
    tip->setWordWrap(true);
    layout->addWidget(tip);

    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("搜索学生姓名...");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &AttendanceSelectDialog::filterRoster);
    layout->addWidget(m_searchEdit);

    m_roster = new QListWidget;
    const auto students = Config::instance().getStudentList();
    for (const auto& s : students) {
        auto* item = new QListWidgetItem(s);
        item->setCheckState(Qt::Unchecked);
        m_roster->addItem(item);
    }
    layout->addWidget(m_roster, 1);

    auto* actions = new QHBoxLayout;
    auto* markAllBtn = new QPushButton("全选缺勤");
    auto* clearAllBtn = new QPushButton("清空勾选");
    auto* allPresentBtn = new QPushButton("全员到齐");
    auto* exportBtn = new QPushButton("导出缺勤");
    auto* saveBtn = new QPushButton("保存");
    auto* cancelBtn = new QPushButton("关闭");
    for (auto* btn : {markAllBtn, clearAllBtn, allPresentBtn, exportBtn, saveBtn, cancelBtn}) {
        btn->setStyleSheet(buttonStylePrimary());
        actions->addWidget(btn);
    }

    connect(markAllBtn, &QPushButton::clicked, [this]() {
        for (int i = 0; i < m_roster->count(); ++i) {
            auto* item = m_roster->item(i);
            if (!item->isHidden()) {
                item->setCheckState(Qt::Checked);
            }
        }
    });
    connect(clearAllBtn, &QPushButton::clicked, [this]() {
        for (int i = 0; i < m_roster->count(); ++i) {
            m_roster->item(i)->setCheckState(Qt::Unchecked);
        }
    });
    connect(allPresentBtn, &QPushButton::clicked, [this]() {
        for (int i = 0; i < m_roster->count(); ++i) {
            m_roster->item(i)->setCheckState(Qt::Unchecked);
        }
        saveSelection();
    });
    connect(exportBtn, &QPushButton::clicked, this, &AttendanceSelectDialog::exportSelection);
    connect(saveBtn, &QPushButton::clicked, this, &AttendanceSelectDialog::saveSelection);
    connect(cancelBtn, &QPushButton::clicked, [this]() { smoothHide(this); });
    layout->addLayout(actions);
}

void AttendanceSelectDialog::filterRoster(const QString& keyword) {
    const QString k = keyword.trimmed();
    for (int i = 0; i < m_roster->count(); ++i) {
        auto* item = m_roster->item(i);
        item->setHidden(!k.isEmpty() && !item->text().contains(k, Qt::CaseInsensitive));
    }
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
        if (m_roster->item(i)->checkState() == Qt::Checked) {
            absentees.append(m_roster->item(i)->text());
        }
    }
    emit saved(absentees);
    hide();
}

void AttendanceSelectDialog::exportSelection() {
    QStringList absentees;
    for (int i = 0; i < m_roster->count(); ++i) {
        if (m_roster->item(i)->checkState() == Qt::Checked) {
            absentees.append(m_roster->item(i)->text());
        }
    }

    const QString path = QFileDialog::getSaveFileName(this, "导出缺勤名单",
                                                      QString("考勤_%1.txt").arg(QDate::currentDate().toString("yyyyMMdd")),
                                                      "Text File (*.txt)");
    if (path.isEmpty()) {
        return;
    }

    QFile out(path);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageBox::warning(this, "导出失败", "无法写入导出文件。");
        return;
    }

    QTextStream stream(&out);
    stream.setCodec("UTF-8");
    stream << "日期：" << QDate::currentDate().toString("yyyy-MM-dd") << "\n";
    stream << "缺勤人数：" << absentees.size() << "\n";
    stream << "缺勤名单：" << (absentees.isEmpty() ? "无" : absentees.join("、")) << "\n";
    QMessageBox::information(this, "导出成功", "缺勤名单已导出。");
}

void AttendanceSelectDialog::closeEvent(QCloseEvent* event) {
    smoothHide(this);
    event->ignore();
}

RandomCallDialog::RandomCallDialog(QWidget* parent) : QDialog(parent) {
    decorateDialog(this, "随机点名");
    setFixedSize(620, 380);

    auto* layout = new QVBoxLayout(this);
    auto* title = new QLabel("随机点名");
    title->setStyleSheet("font-size:22px;font-weight:800;");
    layout->addWidget(title, 0, Qt::AlignHCenter);

    m_nameLabel = new QLabel("准备开始");
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setMinimumHeight(120);
    m_nameLabel->setStyleSheet("font-size:42px;font-weight:900;background:#ffffff;border:1px solid #d8e0eb;border-radius:18px;padding:8px;");
    layout->addWidget(m_nameLabel);

    m_hintLabel = new QLabel("点击“开始点名”后滚动，点击“停止并确定”锁定结果。");
    m_hintLabel->setWordWrap(true);
    layout->addWidget(m_hintLabel);

    m_historyLabel = new QLabel("最近点名：暂无");
    m_historyLabel->setWordWrap(true);
    m_historyLabel->setStyleSheet("background:#ffffff;border:1px solid #e0e7f0;border-radius:10px;padding:8px;");
    layout->addWidget(m_historyLabel);

    auto* row = new QHBoxLayout;
    m_toggleButton = new QPushButton("开始点名");
    m_copyButton = new QPushButton("复制结果");
    m_closeButton = new QPushButton("隐藏窗口");
    for (auto* btn : {m_toggleButton, m_copyButton, m_closeButton}) {
        btn->setMinimumHeight(42);
        btn->setStyleSheet(buttonStylePrimary());
        row->addWidget(btn, 1);
    }
    layout->addLayout(row);

    connect(m_toggleButton, &QPushButton::clicked, this, &RandomCallDialog::toggleRolling);
    connect(m_copyButton, &QPushButton::clicked, [this]() {
        QGuiApplication::clipboard()->setText(m_nameLabel->text());
        m_hintLabel->setText(QString("已复制：%1").arg(m_nameLabel->text()));
    });
    connect(m_closeButton, &QPushButton::clicked, [this]() { smoothHide(this); });

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, [this]() {
        if (m_list.isEmpty()) {
            m_timer->stop();
            m_running = false;
            m_nameLabel->setText("无名单");
            m_toggleButton->setText("开始点名");
            return;
        }
        m_nameLabel->setText(drawName());
        ++m_count;
        if (m_count > 24) {
            m_timer->setInterval(110);
        }
        if (m_count > 34) {
            m_timer->setInterval(180);
        }
    });
}

QString RandomCallDialog::drawName() const {
    const QStringList& pool = (Config::instance().randomNoRepeat && !m_remainingList.isEmpty()) ? m_remainingList : m_list;
    if (pool.isEmpty()) {
        return "无名单";
    }
    return pool[QRandomGenerator::global()->bounded(pool.size())];
}

void RandomCallDialog::toggleRolling() {
    if (!m_running) {
        if (m_list.isEmpty()) {
            m_nameLabel->setText("无名单");
            return;
        }
        m_count = 0;
        m_running = true;
        m_toggleButton->setText("停止并确定");
        m_hintLabel->setText("点名进行中...");
        m_timer->start(45);
        return;
    }

    m_timer->stop();
    m_running = false;
    const QString selected = m_nameLabel->text().trimmed();
    m_toggleButton->setText("再来一次");

    if (!selected.isEmpty() && selected != "无名单") {
        m_history.prepend(selected);
        while (m_history.size() > Config::instance().randomHistorySize) {
            m_history.removeLast();
        }
        m_historyLabel->setText(QString("最近点名：%1").arg(m_history.join("、")));
    }

    if (Config::instance().randomNoRepeat && !selected.isEmpty() && selected != "无名单") {
        m_remainingList.removeAll(selected);
        if (m_remainingList.isEmpty()) {
            m_remainingList = m_list;
            m_hintLabel->setText("本轮已点完全部学生，已自动重置名单。");
        } else {
            m_hintLabel->setText(QString("已确定：%1（剩余 %2 人）").arg(selected).arg(m_remainingList.size()));
        }
    } else {
        m_hintLabel->setText(QString("已确定：%1").arg(selected));
    }
}

void RandomCallDialog::startAnim() {
    m_list = Config::instance().getStudentList();
    m_remainingList = m_list;
    m_running = false;
    m_timer->stop();
    m_toggleButton->setText("开始点名");
    m_historyLabel->setText(m_history.isEmpty() ? "最近点名：暂无" : QString("最近点名：%1").arg(m_history.join("、")));
    if (m_list.isEmpty()) {
        m_nameLabel->setText("无名单");
        m_hintLabel->setText("请先在设置中导入名单");
    } else {
        m_nameLabel->setText("准备开始");
        m_hintLabel->setText(Config::instance().randomNoRepeat ? "当前模式：无重复点名（每轮自动重置）" : "当前模式：允许重复点名");
    }
    smoothShow(this);
}

void RandomCallDialog::closeEvent(QCloseEvent* event) {
    smoothHide(this);
    event->ignore();
}

ClassTimerDialog::ClassTimerDialog(QWidget* parent) : QDialog(parent) {
    decorateDialog(this, "课堂计时器");
    setFixedSize(420, 280);

    auto* layout = new QVBoxLayout(this);
    m_countdownLabel = new QLabel("00:00");
    m_countdownLabel->setAlignment(Qt::AlignCenter);
    m_countdownLabel->setStyleSheet("font-size:54px;font-weight:900;background:#ffffff;border:1px solid #d8e0eb;border-radius:14px;padding:12px;");
    layout->addWidget(m_countdownLabel);

    auto* spinRow = new QHBoxLayout;
    spinRow->addWidget(new QLabel("倒计时（分钟）"));
    m_minutesSpin = new QSpinBox;
    m_minutesSpin->setRange(1, 180);
    m_minutesSpin->setValue(45);
    spinRow->addWidget(m_minutesSpin);
    layout->addLayout(spinRow);

    auto* btnRow = new QHBoxLayout;
    m_startPauseButton = new QPushButton("开始");
    m_resetButton = new QPushButton("重置");
    auto* closeButton = new QPushButton("关闭");
    for (auto* btn : {m_startPauseButton, m_resetButton, closeButton}) {
        btn->setStyleSheet(buttonStylePrimary());
        btnRow->addWidget(btn);
    }
    layout->addLayout(btnRow);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, [this]() {
        if (m_remainingSeconds > 0) {
            --m_remainingSeconds;
            updateCountdownText();
            if (m_remainingSeconds == 0) {
                m_timer->stop();
                m_running = false;
                m_startPauseButton->setText("开始");
                QApplication::beep();
            }
        }
    });

    connect(m_startPauseButton, &QPushButton::clicked, [this]() {
        if (!m_running) {
            if (m_remainingSeconds <= 0) {
                m_remainingSeconds = m_minutesSpin->value() * 60;
            }
            m_running = true;
            m_timer->start(1000);
            m_startPauseButton->setText("暂停");
            return;
        }
        m_running = false;
        m_timer->stop();
        m_startPauseButton->setText("继续");
    });
    connect(m_resetButton, &QPushButton::clicked, [this]() {
        m_timer->stop();
        m_running = false;
        m_remainingSeconds = m_minutesSpin->value() * 60;
        m_startPauseButton->setText("开始");
        updateCountdownText();
    });
    connect(closeButton, &QPushButton::clicked, [this]() { smoothHide(this); });

    m_remainingSeconds = m_minutesSpin->value() * 60;
    updateCountdownText();
}

void ClassTimerDialog::updateCountdownText() {
    const int mm = m_remainingSeconds / 60;
    const int ss = m_remainingSeconds % 60;
    m_countdownLabel->setText(QString("%1:%2").arg(mm, 2, 10, QChar('0')).arg(ss, 2, 10, QChar('0')));
}

void ClassTimerDialog::openTimer() {
    smoothShow(this);
}

void ClassTimerDialog::closeEvent(QCloseEvent* event) {
    smoothHide(this);
    event->ignore();
}

ClassNoteDialog::ClassNoteDialog(QWidget* parent) : QDialog(parent) {
    decorateDialog(this, "课堂便签");
    setFixedSize(480, 360);

    auto* layout = new QVBoxLayout(this);
    m_infoLabel = new QLabel("本地便签（离线存储，不联网）");
    layout->addWidget(m_infoLabel);

    m_editor = new QTextEdit;
    layout->addWidget(m_editor, 1);

    auto* row = new QHBoxLayout;
    auto* saveBtn = new QPushButton("保存便签");
    auto* closeBtn = new QPushButton("关闭");
    saveBtn->setStyleSheet(buttonStylePrimary());
    closeBtn->setStyleSheet(buttonStylePrimary());
    row->addStretch();
    row->addWidget(saveBtn);
    row->addWidget(closeBtn);
    layout->addLayout(row);

    connect(saveBtn, &QPushButton::clicked, this, &ClassNoteDialog::saveNote);
    connect(closeBtn, &QPushButton::clicked, [this]() { smoothHide(this); });
}

void ClassNoteDialog::saveNote() {
    Config::instance().classNote = m_editor->toPlainText();
    Config::instance().save();
    m_infoLabel->setText("本地便签已保存");
}

void ClassNoteDialog::openNote() {
    m_editor->setPlainText(Config::instance().classNote);
    m_infoLabel->setText("本地便签（离线存储，不联网）");
    smoothShow(this);
}

void ClassNoteDialog::closeEvent(QCloseEvent* event) {
    smoothHide(this);
    event->ignore();
}

GroupSplitDialog::GroupSplitDialog(QWidget* parent) : QDialog(parent) {
    decorateDialog(this, "分组抽签");
    setFixedSize(540, 430);

    auto* layout = new QVBoxLayout(this);
    auto* row = new QHBoxLayout;
    row->addWidget(new QLabel("每组人数"));
    m_groupSize = new QSpinBox;
    m_groupSize->setRange(2, 12);
    row->addWidget(m_groupSize);
    auto* generateBtn = new QPushButton("重新分组");
    auto* closeBtn = new QPushButton("关闭");
    generateBtn->setStyleSheet(buttonStylePrimary());
    closeBtn->setStyleSheet(buttonStylePrimary());
    row->addWidget(generateBtn);
    row->addWidget(closeBtn);
    layout->addLayout(row);

    m_result = new QTextEdit;
    m_result->setReadOnly(true);
    layout->addWidget(m_result, 1);

    connect(generateBtn, &QPushButton::clicked, this, &GroupSplitDialog::generate);
    connect(closeBtn, &QPushButton::clicked, [this]() { smoothHide(this); });
}

void GroupSplitDialog::generate() {
    QStringList list = Config::instance().getStudentList();
    if (list.isEmpty()) {
        m_result->setPlainText("暂无学生名单，请先导入。");
        return;
    }

    for (int i = list.size() - 1; i > 0; --i) {
        const int j = QRandomGenerator::global()->bounded(i + 1);
        list.swapItemsAt(i, j);
    }

    const int each = qMax(2, m_groupSize->value());
    QString out;
    int groupIndex = 1;
    for (int i = 0; i < list.size(); i += each) {
        out += QString("第%1组：%2\n").arg(groupIndex++).arg(list.mid(i, each).join("、"));
    }
    m_result->setPlainText(out.trimmed());
}

void GroupSplitDialog::openSplitter() {
    m_groupSize->setValue(Config::instance().groupSplitSize);
    generate();
    smoothShow(this);
}

void GroupSplitDialog::closeEvent(QCloseEvent* event) {
    smoothHide(this);
    event->ignore();
}

ScoreBoardDialog::ScoreBoardDialog(QWidget* parent) : QDialog(parent) {
    decorateDialog(this, "课堂计分板");
    setFixedSize(460, 320);

    auto* layout = new QVBoxLayout(this);
    m_teamALabel = new QLabel;
    m_teamBLabel = new QLabel;
    m_scoreLabel = new QLabel;
    m_scoreLabel->setAlignment(Qt::AlignCenter);
    m_scoreLabel->setStyleSheet("font-size:46px;font-weight:900;background:#ffffff;border:1px solid #d8e0eb;border-radius:14px;padding:8px;");
    layout->addWidget(m_teamALabel);
    layout->addWidget(m_teamBLabel);
    layout->addWidget(m_scoreLabel);

    auto* row = new QHBoxLayout;
    auto* aMinus = new QPushButton("A -1");
    auto* aPlus = new QPushButton("A +1");
    auto* bMinus = new QPushButton("B -1");
    auto* bPlus = new QPushButton("B +1");
    auto* resetBtn = new QPushButton("重置");
    auto* closeBtn = new QPushButton("关闭");
    for (auto* btn : {aMinus, aPlus, bMinus, bPlus, resetBtn, closeBtn}) {
        btn->setStyleSheet(buttonStylePrimary());
        row->addWidget(btn);
    }
    layout->addLayout(row);

    connect(aMinus, &QPushButton::clicked, [this]() { m_scoreA = qMax(0, m_scoreA - 1); refreshScore(); });
    connect(aPlus, &QPushButton::clicked, [this]() { ++m_scoreA; refreshScore(); });
    connect(bMinus, &QPushButton::clicked, [this]() { m_scoreB = qMax(0, m_scoreB - 1); refreshScore(); });
    connect(bPlus, &QPushButton::clicked, [this]() { ++m_scoreB; refreshScore(); });
    connect(resetBtn, &QPushButton::clicked, [this]() { m_scoreA = 0; m_scoreB = 0; refreshScore(); });
    connect(closeBtn, &QPushButton::clicked, [this]() { smoothHide(this); });
}

void ScoreBoardDialog::refreshScore() {
    m_teamALabel->setText(QString("%1").arg(Config::instance().scoreTeamAName));
    m_teamBLabel->setText(QString("%1").arg(Config::instance().scoreTeamBName));
    m_scoreLabel->setText(QString("%1 : %2").arg(m_scoreA).arg(m_scoreB));
}

void ScoreBoardDialog::openBoard() {
    refreshScore();
    smoothShow(this);
}

void ScoreBoardDialog::closeEvent(QCloseEvent* event) {
    smoothHide(this);
    event->ignore();
}


AIAssistantDialog::AIAssistantDialog(QWidget* parent) : QDialog(parent) {
    decorateDialog(this, "AI 助手");
    setFixedSize(700, 560);

    auto* layout = new QVBoxLayout(this);
    auto* title = new QLabel("班级 AI 助手");
    title->setStyleSheet("font-size:22px;font-weight:800;color:#1f4f8f;");
    layout->addWidget(title);

    m_statusLabel = new QLabel("请输入问题后发送（需先在设置中填写硅基流动 API Key）。");
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setStyleSheet("color:#4a647f;");
    layout->addWidget(m_statusLabel);

    m_historyView = new QTextEdit;
    m_historyView->setReadOnly(true);
    m_historyView->setMinimumHeight(300);
    m_historyView->setStyleSheet("background:#ffffff;border:1px solid #d8e0eb;border-radius:14px;padding:12px;");
    layout->addWidget(m_historyView, 1);

    auto* inputCard = new QWidget;
    inputCard->setStyleSheet(cardStyle());
    auto* inputLayout = new QVBoxLayout(inputCard);
    inputLayout->setContentsMargins(10, 10, 10, 10);
    inputLayout->setSpacing(8);

    m_inputEdit = new QTextEdit;
    m_inputEdit->setPlaceholderText("例如：帮我生成一段课堂纪律提醒语...");
    m_inputEdit->setMinimumHeight(92);
    inputLayout->addWidget(m_inputEdit);

    auto* actions = new QHBoxLayout;
    auto* clearBtn = new QPushButton("清空对话");
    m_sendButton = new QPushButton("发送");
    clearBtn->setStyleSheet(buttonStylePrimary());
    m_sendButton->setStyleSheet(buttonStylePrimary());
    actions->addWidget(clearBtn);
    actions->addStretch();
    actions->addWidget(m_sendButton);
    inputLayout->addLayout(actions);
    layout->addWidget(inputCard);

    connect(clearBtn, &QPushButton::clicked, [this]() {
        m_messages = QJsonArray();
        m_historyView->clear();
        m_statusLabel->setText("已清空对话。请输入新问题。");
    });
    connect(m_sendButton, &QPushButton::clicked, this, &AIAssistantDialog::sendMessage);
}

void AIAssistantDialog::appendMessageBubble(const QString& role, const QString& text) {
    const QString safeText = text.toHtmlEscaped().replace("\n", "<br>");
    const bool isUser = (role == "user");
    const QString bubbleColor = isUser ? "#eaf4ff" : "#ffffff";
    const QString align = isUser ? "right" : "left";
    const QString sender = isUser ? "你" : "AI助手";
    m_historyView->append(QString("<div style='text-align:%1;margin:8px 0;'>"
                                  "<div style='display:inline-block;max-width:88%;padding:10px 12px;"
                                  "border:1px solid #d8e0eb;border-radius:12px;background:%2;'>"
                                  "<div style='font-size:12px;color:#56708b;margin-bottom:4px;'>%3</div>%4"
                                  "</div></div>")
                              .arg(align, bubbleColor, sender, safeText));
}

void AIAssistantDialog::sendMessage() {
    const QString userText = m_inputEdit->toPlainText().trimmed();
    if (userText.isEmpty()) {
        return;
    }

    const Config& cfg = Config::instance();
    if (cfg.siliconFlowApiKey.trimmed().isEmpty()) {
        QMessageBox::warning(this, "缺少 API Key", "请先在设置 → 安全与离线 中填写硅基流动 API Key。\n填写后保存即可直接使用。");
        return;
    }

    appendMessageBubble("user", userText);
    m_messages.append(QJsonObject{{"role", "user"}, {"content", userText}});
    m_inputEdit->clear();
    m_sendButton->setEnabled(false);
    m_statusLabel->setText("AI 正在思考中...");

    auto* manager = new QNetworkAccessManager(this);
    QNetworkRequest request(QUrl(cfg.siliconFlowEndpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(cfg.siliconFlowApiKey).toUtf8());

    QJsonArray messages;
    messages.append(QJsonObject{{"role", "system"}, {"content", "你是班级课堂助手，请给出简洁、可执行的建议。"}});
    for (const QJsonValue& v : m_messages) {
        messages.append(v);
    }

    const QJsonObject payload{{"model", cfg.siliconFlowModel},
                              {"messages", messages},
                              {"temperature", 0.6},
                              {"stream", false}};

    QNetworkReply* reply = manager->post(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply, manager]() {
        m_sendButton->setEnabled(true);
        const QByteArray body = reply->readAll();
        if (reply->error() != QNetworkReply::NoError) {
            m_statusLabel->setText(QString("请求失败：%1").arg(reply->errorString()));
            appendMessageBubble("assistant", QString("请求失败：%1\n请检查网络、API Key、模型名。\n返回内容：%2")
                                                .arg(reply->errorString(), QString::fromUtf8(body)));
            reply->deleteLater();
            manager->deleteLater();
            return;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(body);
        QString aiText;
        if (doc.isObject()) {
            const QJsonArray choices = doc.object().value("choices").toArray();
            if (!choices.isEmpty()) {
                aiText = choices.first().toObject().value("message").toObject().value("content").toString().trimmed();
            }
        }

        if (aiText.isEmpty()) {
            aiText = "接口返回为空，请稍后重试。";
        }
        appendMessageBubble("assistant", aiText);
        m_messages.append(QJsonObject{{"role", "assistant"}, {"content", aiText}});
        m_statusLabel->setText("已收到 AI 回复。可以继续提问。");

        reply->deleteLater();
        manager->deleteLater();
    });
}

void AIAssistantDialog::openAssistant() {
    if (m_messages.isEmpty()) {
        appendMessageBubble("assistant", "你好，我可以帮你生成课堂话术、课堂活动设计、班级管理提醒等内容。");
    }
    smoothShow(this);
}

void AIAssistantDialog::closeEvent(QCloseEvent* event) {
    smoothHide(this);
    event->ignore();
}

AddButtonDialog::AddButtonDialog(QWidget* parent) : QDialog(parent) {
    decorateDialog(this, "添加自定义按钮");
    setFixedSize(440, 280);

    auto* layout = new QVBoxLayout(this);

    m_nameEdit = new QLineEdit;
    m_nameEdit->setPlaceholderText("按钮名称");
    layout->addWidget(new QLabel("按钮名称"));
    layout->addWidget(m_nameEdit);

    m_iconEdit = new QLineEdit;
    auto* iconBtn = new QPushButton("选择图标");
    iconBtn->setStyleSheet(buttonStylePrimary());
    connect(iconBtn, &QPushButton::clicked, [this]() {
        const QString p = QFileDialog::getOpenFileName(this, "选择图标", "", "Images (*.png *.jpg *.ico *.svg)");
        if (!p.isEmpty()) {
            m_iconEdit->setText(p);
        }
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
    m_targetEdit->setPlaceholderText("路径 / URL / 功能标识（如 AI_ASSISTANT）");
    layout->addWidget(new QLabel("目标"));
    layout->addWidget(m_targetEdit);
    auto* targetHint = new QLabel("内置功能标识示例：ATTENDANCE、RANDOM_CALL、CLASS_TIMER、CLASS_NOTE、GROUP_SPLIT、SCORE_BOARD、AI_ASSISTANT");
    targetHint->setWordWrap(true);
    targetHint->setStyleSheet("color:#5a6f86;font-size:12px;");
    layout->addWidget(targetHint);

    auto* actions = new QHBoxLayout;
    auto* ok = new QPushButton("确定");
    auto* cancel = new QPushButton("取消");
    ok->setStyleSheet(buttonStylePrimary());
    cancel->setStyleSheet(buttonStylePrimary());
    connect(ok, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    actions->addStretch();
    actions->addWidget(ok);
    actions->addWidget(cancel);
    layout->addLayout(actions);
}

AppButton AddButtonDialog::resultButton() const {
    return {m_nameEdit->text().trimmed(), m_iconEdit->text().trimmed(), m_actionCombo->currentData().toString(),
            m_targetEdit->text().trimmed(), false};
}

FirstRunWizard::FirstRunWizard(QWidget* parent) : QDialog(parent) {
    decorateDialog(this, "欢迎使用 ClassAssistant");
    setFixedSize(560, 460);

    auto* layout = new QVBoxLayout(this);
    auto* intro = new QLabel("首次启动向导：完成基础设置（后续可在设置中修改）。");
    intro->setWordWrap(true);
    layout->addWidget(intro);

    layout->addWidget(new QLabel("悬浮球透明度"));
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

    m_trayClickToOpen = new QCheckBox("托盘单击时展开侧栏");
    m_trayClickToOpen->setChecked(Config::instance().trayClickToOpen);
    layout->addWidget(m_trayClickToOpen);

    m_showAttendanceSummaryOnStart = new QCheckBox("启动时显示考勤概览窗口");
    m_showAttendanceSummaryOnStart->setChecked(Config::instance().showAttendanceSummaryOnStart);
    layout->addWidget(m_showAttendanceSummaryOnStart);

    m_randomNoRepeat = new QCheckBox("随机点名无重复（点完一轮自动重置）");
    m_randomNoRepeat->setChecked(Config::instance().randomNoRepeat);
    layout->addWidget(m_randomNoRepeat);

    m_allowExternalLinks = new QCheckBox("允许打开网络链接（默认关闭）");
    m_allowExternalLinks->setChecked(Config::instance().allowExternalLinks);
    layout->addWidget(m_allowExternalLinks);

    layout->addWidget(new QLabel("默认程序路径（希沃）"));
    m_seewoPathEdit = new QLineEdit(Config::instance().seewoPath);
    auto* browse = new QPushButton("选择程序路径");
    browse->setStyleSheet(buttonStylePrimary());
    connect(browse, &QPushButton::clicked, [this]() {
        const QString p = QFileDialog::getOpenFileName(this, "选择程序", "", "Executable (*.exe);;All Files (*)");
        if (!p.isEmpty()) {
            m_seewoPathEdit->setText(p);
        }
    });
    layout->addWidget(m_seewoPathEdit);
    layout->addWidget(browse);

    auto* done = new QPushButton("完成初始化");
    done->setStyleSheet(buttonStylePrimary());
    connect(done, &QPushButton::clicked, this, &FirstRunWizard::finishSetup);
    layout->addStretch();
    layout->addWidget(done);
}

void FirstRunWizard::finishSetup() {
    auto& cfg = Config::instance();
    cfg.floatingOpacity = m_floatingOpacity->value();
    cfg.attendanceSummaryWidth = m_summaryWidth->value();
    cfg.startCollapsed = m_startCollapsed->isChecked();
    cfg.trayClickToOpen = m_trayClickToOpen->isChecked();
    cfg.showAttendanceSummaryOnStart = m_showAttendanceSummaryOnStart->isChecked();
    cfg.randomNoRepeat = m_randomNoRepeat->isChecked();
    cfg.allowExternalLinks = m_allowExternalLinks->isChecked();
    cfg.seewoPath = m_seewoPathEdit->text().trimmed();
    cfg.firstRunCompleted = true;
    cfg.save();
    accept();
}

void FirstRunWizard::closeEvent(QCloseEvent* event) {
    event->ignore();
}

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    decorateDialog(this, "ClassAssistant 设置");
    setFixedSize(940, 660);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(10);

    auto* content = new QHBoxLayout;
    m_menuTree = new QTreeWidget;
    m_menuTree->setHeaderHidden(true);
    m_menuTree->setFixedWidth(240);

    auto* topDisplay = new QTreeWidgetItem(QStringList{"显示与启动"});
    topDisplay->addChild(new QTreeWidgetItem(QStringList{"窗口与显示"}));
    topDisplay->addChild(new QTreeWidgetItem(QStringList{"启动行为"}));
    topDisplay->addChild(new QTreeWidgetItem(QStringList{"动画与圆角"}));
    auto* topTools = new QTreeWidgetItem(QStringList{"课堂工具"});
    topTools->addChild(new QTreeWidgetItem(QStringList{"点名设置"}));
    topTools->addChild(new QTreeWidgetItem(QStringList{"程序路径"}));
    topTools->addChild(new QTreeWidgetItem(QStringList{"分组与计分"}));
    auto* topData = new QTreeWidgetItem(QStringList{"数据管理"});
    topData->addChild(new QTreeWidgetItem(QStringList{"名单与按钮"}));
    auto* topSafety = new QTreeWidgetItem(QStringList{"安全与离线"});
    topSafety->addChild(new QTreeWidgetItem(QStringList{"联网控制"}));

    m_menuTree->addTopLevelItem(topDisplay);
    m_menuTree->addTopLevelItem(topTools);
    m_menuTree->addTopLevelItem(topData);
    m_menuTree->addTopLevelItem(topSafety);
    m_menuTree->expandAll();

    m_stacked = new QStackedWidget;
    m_stacked->addWidget(createPageDisplayStartup());
    m_stacked->addWidget(createPageClassTools());
    m_stacked->addWidget(createPageDataManagement());
    m_stacked->addWidget(createPageSafety());

    content->addWidget(m_menuTree);
    content->addWidget(m_stacked, 1);
    root->addLayout(content, 1);

    auto* footer = new QHBoxLayout;
    footer->addStretch();
    auto* restore = new QPushButton("还原默认设置");
    auto* save = new QPushButton("保存设置");
    auto* quitAppBtn = new QPushButton("退出应用");
    for (auto* btn : {restore, save, quitAppBtn}) {
        btn->setStyleSheet(buttonStylePrimary());
        btn->setMinimumHeight(38);
        footer->addWidget(btn);
    }
    connect(restore, &QPushButton::clicked, this, &SettingsDialog::restoreDefaults);
    connect(save, &QPushButton::clicked, this, &SettingsDialog::saveData);
    connect(quitAppBtn, &QPushButton::clicked, qApp, &QCoreApplication::quit);
    root->addLayout(footer);

    connect(m_menuTree, &QTreeWidget::currentItemChanged, [this](QTreeWidgetItem* current) {
        if (!current) {
            return;
        }
        const QString label = current->text(0);
        if (label == "显示与启动" || label == "窗口与显示" || label == "启动行为" || label == "动画与圆角") {
            m_stacked->setCurrentIndex(0);
        } else if (label == "课堂工具" || label == "点名设置" || label == "程序路径" || label == "分组与计分") {
            m_stacked->setCurrentIndex(1);
        } else if (label == "数据管理" || label == "名单与按钮") {
            m_stacked->setCurrentIndex(2);
        } else {
            m_stacked->setCurrentIndex(3);
        }
    });

    m_menuTree->setCurrentItem(topDisplay->child(0));
    loadData();
}

QWidget* SettingsDialog::createPageDisplayStartup() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);

    auto* groupDisplay = new QGroupBox("窗口与显示（二级）");
    auto* displayLayout = new QVBoxLayout(groupDisplay);
    displayLayout->addWidget(new QLabel("悬浮球透明度"));
    m_floatingOpacity = new QSlider(Qt::Horizontal);
    m_floatingOpacity->setRange(35, 100);
    displayLayout->addWidget(m_floatingOpacity);

    displayLayout->addWidget(new QLabel("考勤概览宽度"));
    m_summaryWidth = new QSlider(Qt::Horizontal);
    m_summaryWidth->setRange(300, 520);
    displayLayout->addWidget(m_summaryWidth);

    m_compactMode = new QCheckBox("紧凑模式（缩小图标与间距）");
    displayLayout->addWidget(m_compactMode);

    displayLayout->addWidget(new QLabel("主界面宽度"));
    m_sidebarWidth = new QSlider(Qt::Horizontal);
    m_sidebarWidth->setRange(84, 128);
    displayLayout->addWidget(m_sidebarWidth);

    displayLayout->addWidget(new QLabel("过渡动画时长（毫秒）"));
    m_animationDuration = new QSlider(Qt::Horizontal);
    m_animationDuration->setRange(120, 600);
    displayLayout->addWidget(m_animationDuration);

    auto* groupStartup = new QGroupBox("启动行为（二级）");
    auto* startupLayout = new QVBoxLayout(groupStartup);
    m_startCollapsed = new QCheckBox("启动时收起到悬浮球");
    m_trayClickToOpen = new QCheckBox("托盘单击时展开侧栏");
    m_showAttendanceSummaryOnStart = new QCheckBox("启动时显示考勤概览");
    m_collapseHidesToolWindows = new QCheckBox("收起主界面时联动隐藏所有工具窗口");
    startupLayout->addWidget(m_startCollapsed);
    startupLayout->addWidget(m_trayClickToOpen);
    startupLayout->addWidget(m_showAttendanceSummaryOnStart);
    startupLayout->addWidget(m_collapseHidesToolWindows);

    layout->addWidget(groupDisplay);
    layout->addWidget(groupStartup);
    layout->addStretch();
    return page;
}

QWidget* SettingsDialog::createPageClassTools() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);

    auto* groupRandom = new QGroupBox("点名设置（二级）");
    auto* randomLayout = new QVBoxLayout(groupRandom);
    m_randomNoRepeat = new QCheckBox("随机点名无重复（点完一轮自动重置）");
    randomLayout->addWidget(m_randomNoRepeat);

    auto* historyRow = new QHBoxLayout;
    historyRow->addWidget(new QLabel("历史记录条数（三级）"));
    m_historyCount = new QSpinBox;
    m_historyCount->setRange(3, 10);
    historyRow->addWidget(m_historyCount);
    randomLayout->addLayout(historyRow);

    auto* groupPath = new QGroupBox("程序路径（二级）");
    auto* pathLayout = new QHBoxLayout(groupPath);
    m_seewoPathEdit = new QLineEdit;
    auto* choosePath = new QPushButton("选择路径");
    choosePath->setStyleSheet(buttonStylePrimary());
    connect(choosePath, &QPushButton::clicked, [this]() {
        const QString p = QFileDialog::getOpenFileName(this, "选择可执行文件", "", "Executable (*.exe);;All Files (*)");
        if (!p.isEmpty()) {
            m_seewoPathEdit->setText(p);
        }
    });
    pathLayout->addWidget(m_seewoPathEdit, 1);
    pathLayout->addWidget(choosePath);

    auto* groupFeature = new QGroupBox("分组与计分（二级）");
    auto* featureLayout = new QVBoxLayout(groupFeature);
    auto* groupSizeRow = new QHBoxLayout;
    groupSizeRow->addWidget(new QLabel("分组默认人数（三级）"));
    m_groupSize = new QSpinBox;
    m_groupSize->setRange(2, 12);
    groupSizeRow->addWidget(m_groupSize);
    featureLayout->addLayout(groupSizeRow);

    auto* teamARow = new QHBoxLayout;
    teamARow->addWidget(new QLabel("计分队伍A名称（三级）"));
    m_scoreTeamAName = new QLineEdit;
    teamARow->addWidget(m_scoreTeamAName);
    featureLayout->addLayout(teamARow);

    auto* teamBRow = new QHBoxLayout;
    teamBRow->addWidget(new QLabel("计分队伍B名称（三级）"));
    m_scoreTeamBName = new QLineEdit;
    teamBRow->addWidget(m_scoreTeamBName);
    featureLayout->addLayout(teamBRow);

    layout->addWidget(groupRandom);
    layout->addWidget(groupPath);
    layout->addWidget(groupFeature);
    layout->addStretch();
    return page;
}

QWidget* SettingsDialog::createPageDataManagement() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);

    auto* importBtn = new QPushButton("导入班级名单（CSV/TXT）");
    importBtn->setStyleSheet(buttonStylePrimary());
    connect(importBtn, &QPushButton::clicked, this, &SettingsDialog::importStudents);
    layout->addWidget(importBtn);

    layout->addWidget(new QLabel("按钮管理（除“设置”外，主界面按钮都可删改；可一键恢复默认按钮）"));
    m_buttonList = new QListWidget;
    layout->addWidget(m_buttonList, 1);

    auto* btnOps = new QHBoxLayout;
    auto* btnAdd = new QPushButton("添加按钮");
    auto* btnRemove = new QPushButton("删除按钮");
    auto* btnUp = new QPushButton("上移");
    auto* btnDown = new QPushButton("下移");
    auto* btnRestore = new QPushButton("恢复缺失默认按钮");
    for (auto* btn : {btnAdd, btnRemove, btnUp, btnDown, btnRestore}) {
        btn->setStyleSheet(buttonStylePrimary());
        btnOps->addWidget(btn);
    }
    connect(btnAdd, &QPushButton::clicked, this, &SettingsDialog::addButton);
    connect(btnRemove, &QPushButton::clicked, this, &SettingsDialog::removeButton);
    connect(btnUp, &QPushButton::clicked, this, &SettingsDialog::moveUp);
    connect(btnDown, &QPushButton::clicked, this, &SettingsDialog::moveDown);
    connect(btnRestore, &QPushButton::clicked, this, &SettingsDialog::restoreMissingDefaultButtons);
    layout->addLayout(btnOps);

    return page;
}

QWidget* SettingsDialog::createPageSafety() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);

    auto* groupSafety = new QGroupBox("联网控制（二级）");
    auto* safetyLayout = new QVBoxLayout(groupSafety);
    m_allowExternalLinks = new QCheckBox("允许打开 URL 链接（默认关闭，离线推荐保持关闭）");
    safetyLayout->addWidget(m_allowExternalLinks);

    auto* tip = new QLabel("说明：为满足校园离线环境，默认禁用 URL 打开。启用后仅在本机默认浏览器打开链接，不会内置联网代码。");
    tip->setWordWrap(true);
    safetyLayout->addWidget(tip);

    auto* groupAI = new QGroupBox("AI 助手（硅基流动 API）");
    auto* aiLayout = new QVBoxLayout(groupAI);

    auto* keyRow = new QHBoxLayout;
    keyRow->addWidget(new QLabel("API Key"));
    m_apiKeyEdit = new QLineEdit;
    m_apiKeyEdit->setEchoMode(QLineEdit::Password);
    m_apiKeyEdit->setPlaceholderText("sk-...");
    keyRow->addWidget(m_apiKeyEdit, 1);
    aiLayout->addLayout(keyRow);

    auto* modelRow = new QHBoxLayout;
    modelRow->addWidget(new QLabel("模型"));
    m_aiModelEdit = new QLineEdit;
    m_aiModelEdit->setPlaceholderText("Qwen/Qwen3-8B");
    modelRow->addWidget(m_aiModelEdit, 1);
    aiLayout->addLayout(modelRow);

    auto* endpointRow = new QHBoxLayout;
    endpointRow->addWidget(new QLabel("接口地址"));
    m_aiEndpointEdit = new QLineEdit;
    m_aiEndpointEdit->setPlaceholderText("https://api.siliconflow.cn/v1/chat/completions");
    endpointRow->addWidget(m_aiEndpointEdit, 1);
    aiLayout->addLayout(endpointRow);

    auto* tip2 = new QLabel("保存、还原默认、退出应用请使用窗口底部一级操作区。设置好 API Key 并保存后，主界面 AI 按钮可直接使用。");
    tip2->setWordWrap(true);

    layout->addWidget(groupSafety);
    layout->addWidget(groupAI);
    layout->addWidget(tip2);
    layout->addStretch();
    return page;
}

void SettingsDialog::loadData() {
    const auto& cfg = Config::instance();
    m_floatingOpacity->setValue(cfg.floatingOpacity);
    m_summaryWidth->setValue(cfg.attendanceSummaryWidth);
    m_startCollapsed->setChecked(cfg.startCollapsed);
    m_trayClickToOpen->setChecked(cfg.trayClickToOpen);
    m_showAttendanceSummaryOnStart->setChecked(cfg.showAttendanceSummaryOnStart);
    m_collapseHidesToolWindows->setChecked(cfg.collapseHidesToolWindows);
    m_compactMode->setChecked(cfg.compactMode);
    m_sidebarWidth->setValue(cfg.sidebarWidth);
    m_animationDuration->setValue(cfg.animationDurationMs);
    m_randomNoRepeat->setChecked(cfg.randomNoRepeat);
    m_historyCount->setValue(cfg.randomHistorySize);
    m_allowExternalLinks->setChecked(cfg.allowExternalLinks);
    m_groupSize->setValue(cfg.groupSplitSize);
    m_scoreTeamAName->setText(cfg.scoreTeamAName);
    m_scoreTeamBName->setText(cfg.scoreTeamBName);
    m_seewoPathEdit->setText(cfg.seewoPath);
    m_apiKeyEdit->setText(cfg.siliconFlowApiKey);
    m_aiModelEdit->setText(cfg.siliconFlowModel);
    m_aiEndpointEdit->setText(cfg.siliconFlowEndpoint);

    m_buttonList->clear();
    for (const auto& b : cfg.getButtons()) {
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
    if (!item) {
        return;
    }

    delete item;
}

void SettingsDialog::restoreMissingDefaultButtons() {
    QSet<QString> existingTargets;
    for (int i = 0; i < m_buttonList->count(); ++i) {
        existingTargets.insert(m_buttonList->item(i)->data(Qt::UserRole + 3).toString());
    }

    int restored = 0;
    for (const AppButton& button : Config::defaultButtons()) {
        if (existingTargets.contains(button.target)) {
            continue;
        }
        auto* item = new QListWidgetItem(QString("%1 [%2]").arg(button.name, button.action));
        item->setData(Qt::UserRole, button.name);
        item->setData(Qt::UserRole + 1, button.iconPath);
        item->setData(Qt::UserRole + 2, button.action);
        item->setData(Qt::UserRole + 3, button.target);
        item->setData(Qt::UserRole + 4, button.isSystem);
        m_buttonList->addItem(item);
        ++restored;
    }

    QMessageBox::information(this,
                             "完成",
                             restored > 0 ? QString("已恢复 %1 个默认按钮。\n记得点击保存设置。").arg(restored)
                                          : "当前没有缺失的默认按钮。");
}

void SettingsDialog::moveUp() {
    const int row = m_buttonList->currentRow();
    if (row <= 0) {
        return;
    }
    auto* item = m_buttonList->takeItem(row);
    m_buttonList->insertItem(row - 1, item);
    m_buttonList->setCurrentRow(row - 1);
}

void SettingsDialog::moveDown() {
    const int row = m_buttonList->currentRow();
    if (row < 0 || row >= m_buttonList->count() - 1) {
        return;
    }
    auto* item = m_buttonList->takeItem(row);
    m_buttonList->insertItem(row + 1, item);
    m_buttonList->setCurrentRow(row + 1);
}

void SettingsDialog::saveData() {
    auto& cfg = Config::instance();
    cfg.floatingOpacity = m_floatingOpacity->value();
    cfg.attendanceSummaryWidth = m_summaryWidth->value();
    cfg.startCollapsed = m_startCollapsed->isChecked();
    cfg.trayClickToOpen = m_trayClickToOpen->isChecked();
    cfg.showAttendanceSummaryOnStart = m_showAttendanceSummaryOnStart->isChecked();
    cfg.collapseHidesToolWindows = m_collapseHidesToolWindows->isChecked();
    cfg.compactMode = m_compactMode->isChecked();
    cfg.sidebarWidth = m_sidebarWidth->value();
    cfg.animationDurationMs = m_animationDuration->value();
    cfg.randomNoRepeat = m_randomNoRepeat->isChecked();
    cfg.randomHistorySize = m_historyCount->value();
    cfg.allowExternalLinks = m_allowExternalLinks->isChecked();
    cfg.groupSplitSize = m_groupSize->value();
    cfg.scoreTeamAName = m_scoreTeamAName->text().trimmed().isEmpty() ? QString("红队") : m_scoreTeamAName->text().trimmed();
    cfg.scoreTeamBName = m_scoreTeamBName->text().trimmed().isEmpty() ? QString("蓝队") : m_scoreTeamBName->text().trimmed();
    cfg.seewoPath = m_seewoPathEdit->text().trimmed();
    cfg.siliconFlowApiKey = m_apiKeyEdit->text().trimmed();
    cfg.siliconFlowModel = m_aiModelEdit->text().trimmed().isEmpty() ? QString("Qwen/Qwen3-8B") : m_aiModelEdit->text().trimmed();
    cfg.siliconFlowEndpoint = m_aiEndpointEdit->text().trimmed().isEmpty() ? QString("https://api.siliconflow.cn/v1/chat/completions")
                                                                            : m_aiEndpointEdit->text().trimmed();

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
    smoothHide(this);
}

void SettingsDialog::restoreDefaults() {
    if (QMessageBox::question(this, "恢复默认", "确定恢复默认设置和默认按钮/名单吗？") != QMessageBox::Yes) {
        return;
    }

    Config::instance().resetToDefaults(true);
    loadData();
    emit configChanged();
}

void SettingsDialog::closeEvent(QCloseEvent* event) {
    smoothHide(this);
    event->ignore();
}
