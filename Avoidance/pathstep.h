#ifndef PATHSTEP_H
#define PATHSTEP_H

#include <QPointF>
#include <QPen>
#include <QGraphicsPathItem>

struct PathStep
{
    using ui_item_t = std::shared_ptr<QGraphicsPathItem>;
    PathStep(const QPointF s, const QPointF c1, const QPointF c2, const QPointF g, qreal width) : start(s), checkPoint1(c1), checkPoint2(c2), goal(g) {
        QPainterPath bezier;
        bezier.moveTo(s);
        bezier.cubicTo(checkPoint1, checkPoint2, g);

        uiItem = std::make_shared<QGraphicsPathItem>(bezier);
        QPen pen(Qt::blue, width);
        uiItem->setPen(pen);
        uiItem->setOpacity(0.3);
    }

    PathStep(QPointF s, QPointF g, qreal width) : start(s), checkPoint1(s), checkPoint2(g), goal(g) {
        QPainterPath bezier;
        bezier.moveTo(s);
        bezier.cubicTo(checkPoint1, checkPoint2, g);

        uiItem = std::make_shared<QGraphicsPathItem>(bezier);
        QPen pen(Qt::blue, width);
        uiItem->setPen(pen);
        uiItem->setOpacity(0.3);
    }

    void setStart(const QPointF s)
    {
        if (start == checkPoint1) {
            checkPoint1 = s;
            start = s;
        }

        QPainterPath bezier;
        bezier.moveTo(start);
        bezier.cubicTo(checkPoint1, checkPoint2, goal);
        uiItem->setPath(bezier);
    }

    QPointF start;
    QPointF checkPoint1;
    QPointF checkPoint2;
    QPointF goal;

    std::shared_ptr<QGraphicsPathItem> uiItem;
};

#endif // PATHSTEP_H
