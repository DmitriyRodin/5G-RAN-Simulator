#include "map_widget.hpp"

#include <cmath>
#include <limits>

#include <QMouseEvent>
#include <QPainter>
#include <QTimer>
#include <QWheelEvent>
#include <QtMath>

#include "controller/simulation_controller.hpp"
#include "gnb_logic.hpp"
#include "ue_logic.hpp"

MapWidget::MapWidget(std::shared_ptr<SimulationController> controller,
                     QWidget* parent)
    : QWidget(parent)
    , controller_(controller)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    setMinimumSize(600, 750);
    setMaximumSize(800, 750);

    scale_ = 0.1;
    camera_offset_ = QPointF(0, 0);

    refresh_timer_ = new QTimer(this);
    connect(refresh_timer_, &QTimer::timeout, this,
            QOverload<>::of(&MapWidget::update));
    refresh_timer_->start(33);
}

void MapWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), Qt::white);

    if (!controller_) {
        return;
    }

    p.translate(width() / 2.0, height() / 2.0);
    p.scale(scale_, scale_);
    p.translate(camera_offset_);

    drawGrid(p);

    for (auto gnb : controller_->getGnbs()) {
        if (gnb) {
            drawGnb(p, gnb->getId(), gnb->position(), gnb->getRadius());
        }
    }

    for (auto ue : controller_->getUes()) {
        if (ue) {
            drawUe(p, ue->getId(), ue->position(), ue->isConnected());
        }
    }
}

void MapWidget::drawGrid(QPainter& p)
{
    p.save();
    QPen gridPen(QColor(235, 235, 235), 1);
    gridPen.setCosmetic(true);
    p.setPen(gridPen);

    const int step = 200;
    const int range = 10000;

    for (int i = -range; i <= range; i += step) {
        p.drawLine(i, -range, i, range);
        p.drawLine(-range, i, range, i);
    }
    p.restore();
}

void MapWidget::drawGnb(QPainter& p, uint32_t id, const QPointF& pos,
                        double radius)
{
    QPen boundary_pen(QColor(0, 100, 255, 150), 1, Qt::DashLine);

    boundary_pen.setCosmetic(true);
    p.setPen(boundary_pen);
    p.setBrush(QColor(0, 100, 255, 20));
    p.drawEllipse(pos, radius, radius);
    double icon_size = 12.0 / scale_;
    p.setPen(QPen(Qt::black, 1));
    p.setBrush(Qt::blue);

    QPolygonF triangle;
    triangle << pos + QPointF(0, -icon_size)
             << pos + QPointF(-icon_size * 0.8, icon_size * 0.8)
             << pos + QPointF(icon_size * 0.8, icon_size * 0.8);
    p.drawPolygon(triangle);

    p.save();
    p.resetTransform();
    const QPointF screen_pos = (pos + camera_offset_) * scale_ +
                               QPointF(width() / 2.0, height() / 2.0);
    p.drawText(screen_pos + QPointF(15, -15), QString("gNB-%1").arg(id));
    p.restore();
}

void MapWidget::drawUe(QPainter& p, uint32_t id, const QPointF& pos,
                       bool connected)
{
    double r = 6.0 / scale_;
    p.setBrush(connected ? Qt::green : Qt::red);
    p.setPen(Qt::black);
    p.drawEllipse(pos, r, r);
}

void MapWidget::wheelEvent(QWheelEvent* event)
{
    double delta = (event->angleDelta().y() > 0) ? 1.15 : 0.85;
    scale_ *= delta;
    scale_ = qBound(0.001, scale_, 100.0);
    update();
}

void MapWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton) {
        last_mouse_pos_ = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
}

void MapWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::RightButton) {
        QPoint delta = event->pos() - last_mouse_pos_;

        camera_offset_ += QPointF(delta.x() / scale_, delta.y() / scale_);

        last_mouse_pos_ = event->pos();
        update();
    }
}

void MapWidget::mouseReleaseEvent(QMouseEvent* event)
{
    setCursor(Qt::ArrowCursor);
}
