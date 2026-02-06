#include "floatingballwindow.h"

#include <QMouseEvent>
#include <QPainter>

FloatingBallWindow::FloatingBallWindow(QWidget *parent) : QWidget(parent) {
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setFixedSize(58, 58);
    move(1200, 680);
}

void FloatingBallWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
    }
}

void FloatingBallWindow::mouseMoveEvent(QMouseEvent *event) {
    if (m_dragging) {
        move(event->globalPos() - m_dragPosition);
    }
}

void FloatingBallWindow::mouseReleaseEvent(QMouseEvent *event) {
    if (m_dragging && rect().contains(event->pos())) {
        emit clicked();
    }
    m_dragging = false;
}

void FloatingBallWindow::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#3B82F6"));
    painter.drawEllipse(rect().adjusted(3, 3, -3, -3));
    painter.setPen(Qt::white);
    painter.drawText(rect(), Qt::AlignCenter, "CA");
}
