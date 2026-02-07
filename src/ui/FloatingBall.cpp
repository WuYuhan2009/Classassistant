#include "FloatingBall.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>

FloatingBall::FloatingBall(QWidget* parent) : QWidget(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(60, 60);

    const QRect screen = QApplication::primaryScreen()->availableGeometry();
    move(screen.width() - 80, screen.height() - 100);
}

void FloatingBall::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(QColor(0, 120, 215, 200));
    p.setPen(Qt::NoPen);
    p.drawEllipse(5, 5, 50, 50);

    p.setPen(Qt::white);
    QFont f = font();
    f.setBold(true);
    p.setFont(f);
    p.drawText(rect(), Qt::AlignCenter, "展开");
}

void FloatingBall::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        m_dragPos = e->globalPos() - frameGeometry().topLeft();
        m_isDragging = false;
    }
}

void FloatingBall::mouseMoveEvent(QMouseEvent* e) {
    if (e->buttons() & Qt::LeftButton) {
        move(e->globalPos() - m_dragPos);
        m_isDragging = true;
    }
}

void FloatingBall::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton && !m_isDragging) {
        emit clicked();
    }
}
