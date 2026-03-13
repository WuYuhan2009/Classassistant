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

    const QRect outer(1, 1, width() - 2, height() - 2);
    p.setPen(QPen(QColor(180, 188, 198), 1));
    p.setBrush(QColor(250, 252, 255));
    p.drawEllipse(outer);

    const int innerMargin = width() / 5;
    const QRect inner(innerMargin, innerMargin, width() - innerMargin * 2, height() - innerMargin * 2);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(224, 232, 242));
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
