#pragma once

#include <QWidget>

class FloatingBall : public QWidget {
    Q_OBJECT
public:
    explicit FloatingBall(QWidget* parent = nullptr);

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QPoint m_dragPos;
    bool m_isDragging = false;
};
