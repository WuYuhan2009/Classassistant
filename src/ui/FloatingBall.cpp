#include "FloatingBall.h"

#include "../Utils.h"
#include "FluentTheme.h"

#include <QApplication>
#include <QCloseEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QTouchEvent>
#include <QGraphicsDropShadowEffect>

FloatingBall::FloatingBall(QWidget* parent) : QWidget(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_AcceptTouchEvents);
    const int size = Config::instance().floatingBallSize;
    setFixedSize(size, size);
    setWindowOpacity(Config::instance().floatingOpacity / 100.0);
    auto* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(22);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0, 0, 0, 95));
    setGraphicsEffect(shadow);
    restoreSavedPosition();
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
            snapToScreenEdge();
        }
        event->accept();
        return true;
    }
    return QWidget::event(event);
}

void FloatingBall::moveToBottomRight() {
    moveToDefaultCollapsedPosition();
}

void FloatingBall::moveToDefaultCollapsedPosition() {
    const QRect screen = QApplication::primaryScreen()->availableGeometry();
    const int x = screen.right() - width() - 12;
    const int y = screen.top() + ((screen.height() * 2) / 3) - height() / 2;
    move(x, qBound(screen.top() + 8, y, screen.bottom() - height() - 8));
    emit positionCommitted(pos());
}

void FloatingBall::restoreSavedPosition() {
    const QRect screen = QApplication::primaryScreen()->availableGeometry();
    auto& cfg = Config::instance();
    if (cfg.floatingBallX < 0 || cfg.floatingBallY < 0) {
        moveToDefaultCollapsedPosition();
        return;
    }

    const int x = qBound(screen.left() + 8, cfg.floatingBallX, screen.right() - width() - 8);
    const int y = qBound(screen.top() + 8, cfg.floatingBallY, screen.bottom() - height() - 8);
    move(x, y);
}

void FloatingBall::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QRectF outer(2, 2, width() - 4, height() - 4);
    p.setPen(QPen(QColor(72, 72, 76, 185), 1.1));
    p.setBrush(QColor(210, 210, 214, 220));
    p.drawEllipse(outer);

    const qreal ringInset = width() * 0.2;
    const QRectF ring(outer.adjusted(ringInset, ringInset, -ringInset, -ringInset));
    p.setPen(QPen(QColor(86, 88, 94, 215), 5));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(ring);

    const qreal coreInset = width() * 0.34;
    const QRectF core(outer.adjusted(coreInset, coreInset, -coreInset, -coreInset));
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(245, 245, 247, 235));
    p.drawEllipse(core);
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
        snapToScreenEdge();
    }
}

void FloatingBall::snapToScreenEdge() {
    const QRect screen = QApplication::primaryScreen()->availableGeometry();
    const int snapX = x() + width() / 2 < screen.center().x() ? screen.left() + 8 : screen.right() - width() - 8;
    const int boundedY = qBound(screen.top() + 8, y(), screen.bottom() - height() - 8);
    move(snapX, boundedY);
    emit positionCommitted(pos());
}

void FloatingBall::closeEvent(QCloseEvent* event) {
    if (AppState::isQuitting()) {
        event->accept();
        return;
    }
    hide();
    event->ignore();
}
