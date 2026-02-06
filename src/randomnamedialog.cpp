#include "randomnamedialog.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>

RandomNameDialog::RandomNameDialog(const QString &pickedName, QWidget *parent)
    : QDialog(parent), m_nameLabel(new QLabel(pickedName, this)), m_timer(new QTimer(this)) {
    setWindowTitle("课堂随机点名");
    resize(360, 220);
    auto *layout = new QVBoxLayout(this);

    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setStyleSheet("font-weight:bold;color:#38BDF8;font-size:18px;");

    layout->addStretch();
    layout->addWidget(m_nameLabel);
    layout->addStretch();

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    layout->addWidget(buttons);

    connect(m_timer, &QTimer::timeout, this, &RandomNameDialog::animate);
    m_timer->start(60);
}

void RandomNameDialog::animate() {
    m_size += 1;
    if (m_size > 36) m_size = 18;
    m_nameLabel->setStyleSheet(QString("font-weight:bold;color:#38BDF8;font-size:%1px;").arg(m_size));
}
