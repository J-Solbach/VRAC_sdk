#pragma once

#include <QPointF>
#include <QPen>
#include <QGraphicsPathItem>

struct path_step
{
    using ui_item_t = std::shared_ptr<QGraphicsPathItem>;
    path_step(const QPointF s, const QPointF c1, const QPointF c2, const QPointF g, qreal width) : start(s), check_point_1(c1), check_point_2(c2), goal(g) {
        QPainterPath bezier;
        bezier.moveTo(s);
        bezier.cubicTo(check_point_1, check_point_2, g);

        ui_item = std::make_shared<QGraphicsPathItem>(bezier);
        QPen pen(Qt::blue, width);
        ui_item->setPen(pen);
        ui_item->setOpacity(0.3);
    }

    path_step(QPointF s, QPointF g, qreal width) : start(s), check_point_1(s), check_point_2(g), goal(g) {
        QPainterPath bezier;
        bezier.moveTo(s);
        bezier.cubicTo(check_point_1, check_point_2, g);

        ui_item = std::make_shared<QGraphicsPathItem>(bezier);
        QPen pen(Qt::blue, width);
        ui_item->setPen(pen);
        ui_item->setOpacity(0.3);
    }

    void setStart(const QPointF s)
    {
        if (start == check_point_1) {
            check_point_1 = s;
            start = s;
        }

        QPainterPath bezier;
        bezier.moveTo(start);
        bezier.cubicTo(check_point_1, check_point_2, goal);
        ui_item->setPath(bezier);
    }

    QPointF start;
    QPointF check_point_1;
    QPointF check_point_2;
    QPointF goal;

    std::shared_ptr<QGraphicsPathItem> ui_item;
};
