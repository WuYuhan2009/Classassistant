#include "FloatingBall.h"

#include <QApplication>
#include <QCloseEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>

FloatingBall::FloatingBall(QWidget* parent) : QWidget(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(64, 64);

    const QRect screen = QApplication::primaryScreen()->availableGeometry();
    move(screen.width() - 90, screen.height() - 110);
}

void FloatingBall::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(QColor(0, 120, 215, 210));
    p.setPen(Qt::NoPen);
    p.drawEllipse(4, 4, 56, 56);

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

void FloatingBall::closeEvent(QCloseEvent* event) {
    hide();
    event->ignore();
}
