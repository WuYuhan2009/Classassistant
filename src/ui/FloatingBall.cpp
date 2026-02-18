#include "FloatingBall.h"

#include "../Utils.h"

#include <QApplication>
#include <QCloseEvent>
#include <QIcon>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QTouchEvent>

FloatingBall::FloatingBall(QWidget* parent) : QWidget(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_AcceptTouchEvents);
    setFixedSize(70, 70);
    setWindowOpacity(Config::instance().floatingOpacity / 100.0);
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
            const int x = screen.right() - width() - 8;
            const int boundedY = qBound(screen.top() + 8, y(), screen.bottom() - height() - 8);
            move(x, boundedY);
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

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(73, 69, 79, 36));
    p.drawEllipse(8, 12, 54, 54);

    p.setBrush(QColor(234, 221, 255));
    p.drawEllipse(3, 3, 64, 64);

    QLinearGradient fill(8, 8, 60, 60);
    fill.setColorAt(0.0, QColor(103, 80, 164));
    fill.setColorAt(1.0, QColor(98, 91, 113));
    p.setBrush(fill);
    p.setPen(QPen(QColor(255, 255, 255, 180), 1));
    p.drawEllipse(8, 8, 54, 54);

    const QString expandIconPath = Config::instance().resolveIconPath("icon_expand.png");
    QIcon icon(expandIconPath);
    if (!icon.isNull()) {
        icon.paint(&p, QRect(17, 17, 36, 36));
    } else {
        p.setPen(QColor(255, 251, 254));
        QFont f = font();
        f.setBold(true);
        f.setPointSize(10);
        p.setFont(f);
        p.drawText(QRect(8, 8, 54, 54), Qt::AlignCenter, "展开");
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
        const int boundedY = qBound(screen.top() + 8, y(), screen.bottom() - height() - 8);
        move(x, boundedY);
    }
}

void FloatingBall::closeEvent(QCloseEvent* event) {
    hide();
    event->ignore();
}
