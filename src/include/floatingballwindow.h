#pragma once

#include <QWidget>

class FloatingBallWindow : public QWidget {
    Q_OBJECT
public:
    explicit FloatingBallWindow(QWidget *parent = nullptr);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QPoint m_dragPosition;
    bool m_dragging = false;
};
