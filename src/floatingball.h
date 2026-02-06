#ifndef FLOATINGBALL_H
#define FLOATINGBALL_H

#include <QWidget>
#include <QPoint>

class FloatingBall : public QWidget
{
    Q_OBJECT
public:
    explicit FloatingBall(QWidget *parent = nullptr);

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QPoint m_dragPosition;
    bool m_dragging;
    bool m_hovered;
    bool m_moved;

    void loadPosition();
    void savePosition();
};

#endif
