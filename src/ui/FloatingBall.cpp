#include "FloatingBall.h"

#include "../Utils.h"

#include <QApplication>
#include <QCloseEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>

FloatingBall::FloatingBall(QWidget* parent) : QWidget(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(70, 70);
    setWindowOpacity(Config::instance().floatingOpacity / 100.0);
    moveToBottomRight();
}

void FloatingBall::moveToBottomRight() {
    const QRect screen = QApplication::primaryScreen()->availableGeometry();
    move(screen.right() - width() - 14, screen.bottom() - height() - 14);
}

void FloatingBall::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    p.setBrush(QColor(0, 123, 255, 220));
    p.setPen(QPen(QColor(255, 255, 255, 170), 2));
    p.drawEllipse(3, 3, 64, 64);

    QIcon icon(":/assets/icon_expand.png");
    if (!icon.isNull()) {
        icon.paint(&p, QRect(16, 16, 38, 38));
    } else {
        p.setPen(Qt::white);
        QFont f = font();
        f.setBold(true);
        f.setPointSize(12);
        p.setFont(f);
        p.drawText(rect(), Qt::AlignCenter, "展开");
    }
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
        return;
    }
    if (e->button() == Qt::LeftButton && m_isDragging) {
        const QRect screen = QApplication::primaryScreen()->availableGeometry();
        const int x = screen.right() - width() - 8;
        const int y = qBound(screen.top() + 8, y(), screen.bottom() - height() - 8);
        move(x, y);
    }
}

void FloatingBall::closeEvent(QCloseEvent* event) {
    hide();
    event->ignore();
}
