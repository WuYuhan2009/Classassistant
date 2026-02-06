#include "floatingball.h"

#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QScreen>
#include <QSettings>

FloatingBall::FloatingBall(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool)
    , m_dragging(false), m_hovered(false), m_moved(false) {
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(60, 60);
    loadPosition();
}

void FloatingBall::loadPosition() {
    QSettings settings;
    QPoint p = settings.value("FloatingBall/Position", QPoint(-1, -1)).toPoint();
    if (p == QPoint(-1, -1)) {
        QRect r = QApplication::primaryScreen()->availableGeometry();
        p = QPoint(r.right() - 80, r.bottom() - 80);
    }
    move(p);
}

void FloatingBall::savePosition() {
    QSettings settings;
    settings.setValue("FloatingBall/Position", pos());
}

void FloatingBall::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(m_hovered ? QColor(100,150,255,230) : QColor(70,130,255,200));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(rect().adjusted(5,5,-5,-5));
    painter.setPen(Qt::white);
    QFont f = painter.font();
    f.setPointSize(10); f.setBold(true); painter.setFont(f);
    painter.drawText(rect(), Qt::AlignCenter, "CA");
}

void FloatingBall::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_moved = false;
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void FloatingBall::mouseMoveEvent(QMouseEvent *event) {
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        m_moved = true;
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

void FloatingBall::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (!m_moved) emit clicked();
        m_dragging = false;
        savePosition();
        event->accept();
    }
}

void FloatingBall::enterEvent(QEvent *) { m_hovered = true; update(); }
void FloatingBall::leaveEvent(QEvent *) { m_hovered = false; update(); }
