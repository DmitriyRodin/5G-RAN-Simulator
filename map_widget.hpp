#ifndef MAP_WIDGET_H
#define MAP_WIDGET_H

#include <memory>

#include <QTimer>
#include <QWidget>

class SimulationController;

class MapWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MapWidget(std::shared_ptr<SimulationController> controller,
                       QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

    void drawGrid(QPainter& p);
    void drawGnb(QPainter& p, uint32_t id, const QPointF& pos, double radius);
    void drawUe(QPainter& p, uint32_t id, const QPointF& pos, bool connected);

private:
    std::shared_ptr<SimulationController> controller_;

    double scale_ = 1.0;
    QTimer* refresh_timer_{nullptr};
    QPoint last_mouse_pos_;

    QPointF camera_offset_ = {0, 0};
};

#endif  // MAP_WIDGET_H
