#pragma once

#include <QWidget>

class QEvent;
class QTouchEvent;

class FloatingBall : public QWidget {
    Q_OBJECT
public:
    explicit FloatingBall(QWidget* parent = nullptr);
    void moveToBottomRight();
    void moveToDefaultCollapsedPosition();
    void restoreSavedPosition();

signals:
    void clicked();
    void positionCommitted(const QPoint& globalTopLeft);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    bool event(QEvent* event) override;

private:
    void snapToScreenEdge();
    QPoint m_dragPos;
    QPoint m_touchStartPos;
    bool m_isDragging = false;
};
