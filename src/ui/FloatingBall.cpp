#include "FloatingBall.h"

#include "../Utils.h"
#include "FluentTheme.h"

#include <QApplication>
#include <QCloseEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QTouchEvent>

FloatingBall::FloatingBall(QWidget* parent) : QWidget(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_AcceptTouchEvents);
    const int size = Config::instance().floatingBallSize;
    setFixedSize(size, size);
    setWindowOpacity(Config::instance().floatingOpacity / 100.0);
    FluentTheme::applyWinUIWindowShadow(this);
    moveToBottomRight();
}

bool FloatingBall::event(QEvent* event) {
    if (event->type() == QEvent::TouchBegin
        || event->type() == QEvent::TouchUpdate
        || event->type() == QEvent::TouchEnd) {
        auto* touchEvent = static_cast<QTouchEvent*>(event);
        if (touchEvent->touchPoints().isEmpty()) {
            return QWidget::event(event);
        }

        const auto& point = touchEvent->touchPoints().first();
        if (event->type() == QEvent::TouchBegin) {
            m_touchStartPos = point.screenPos().toPoint();
            m_dragPos = m_touchStartPos - frameGeometry().topLeft();
            m_isDragging = false;
            event->accept();
            return true;
        }

        if (event->type() == QEvent::TouchUpdate) {
            const QPoint touchPos = point.screenPos().toPoint();
            if ((touchPos - m_touchStartPos).manhattanLength() > 8) {
                m_isDragging = true;
            }
            move(touchPos - m_dragPos);
            event->accept();
            return true;
        }

        if (!m_isDragging) {
            emit clicked();
        } else {
            const QRect screen = QApplication::primaryScreen()->availableGeometry();
            const int snapX = x() + width() / 2 < screen.center().x() ? screen.left() + 8 : screen.right() - width() - 8;
            const int boundedY = qBound(screen.top() + 8, y(), screen.bottom() - height() - 8);
            move(snapX, boundedY);
        }
        event->accept();
        return true;
    }
    return QWidget::event(event);
}

void FloatingBall::moveToBottomRight() {
    const QRect screen = QApplication::primaryScreen()->availableGeometry();
    move(screen.right() - width() - 14, screen.bottom() - height() - 14);
}

void FloatingBall::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QRect ringRect(2, 2, width() - 4, height() - 4);
    QRadialGradient outerGlow(ringRect.center(), width() / 2.0);
    outerGlow.setColorAt(0.0, QColor(255, 255, 255, 245));
    outerGlow.setColorAt(0.75, QColor(208, 214, 223, 230));
    outerGlow.setColorAt(1.0, QColor(48, 54, 62, 220));
    p.setPen(Qt::NoPen);
    p.setBrush(outerGlow);
    p.drawEllipse(ringRect);

    const int innerMargin = width() / 6;
    QRect inner(innerMargin, innerMargin, width() - innerMargin * 2, height() - innerMargin * 2);
    p.setBrush(QColor(245, 245, 240));
    p.setPen(QPen(QColor(32, 35, 40, 160), 1.5));
    p.drawEllipse(inner);
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
        const int snapX = x() + width() / 2 < screen.center().x() ? screen.left() + 8 : screen.right() - width() - 8;
        const int boundedY = qBound(screen.top() + 8, y(), screen.bottom() - height() - 8);
        move(snapX, boundedY);
    }
}

void FloatingBall::closeEvent(QCloseEvent* event) {
    if (AppState::isQuitting()) {
        event->accept();
        return;
    }
    hide();
    event->ignore();
}
