#pragma once

#include <QPolygonF>
#include <QGraphicsPolygonItem>
#include <QPen>

struct Obstacle
{
    Obstacle(QPointF centerPos, int radius, qreal robot_width, const QVector<QPointF>& outer= QVector<QPointF>())
    {
        poly = outer;
        // can be reduced to improve calculation time but will reduce precision of the avoidance
        poly.append(generatePolygon(centerPos, radius));

        uiItem = std::make_shared<QGraphicsPolygonItem>(poly);
        uiItem->setFlags(QGraphicsItem::ItemIsMovable);
        uiItemAvoidance = std:: make_shared<QGraphicsPolygonItem>();
        uiItemAvoidance->setParentItem(uiItem.get());

        polyAvoidance = generatePolygon(centerPos, radius + robot_width * 0.8);
        uiItemAvoidance->setPolygon(polyAvoidance);

        uiItem->setBrush(QBrush(Qt::black));
        uiItem->setOpacity(0.5);
        uiItemAvoidance->setPen(QPen(Qt::green, 20));
    }

    Obstacle(QPolygonF poly, QPolygonF polyAvoidance = QPolygonF())
    {
        uiItem = std::make_shared<QGraphicsPolygonItem>(poly);

        if (polyAvoidance.isEmpty()) return;
        uiItemAvoidance = std::make_shared<QGraphicsPolygonItem>(polyAvoidance);
        uiItemAvoidance->setParentItem(uiItem.get());
    }

    static QPolygonF generatePolygon(QPointF centerPos, int radius)
    {
        QPolygonF octogon;
        const int vertices = 8;
        for(int i=0; i< vertices; i++)
        {
            QPointF octoPoint;
            octoPoint.setX(centerPos.x() + radius * cos((i + 0.5) * 2 * 3.1415 / vertices));
            octoPoint.setY(centerPos.y() + radius * sin( (i + 0.5) * 2 * 3.1415 / vertices));
            octogon.append(octoPoint);
        }

        return octogon;
    }

    QPolygonF poly;
    QPolygonF polyAvoidance;
    std::shared_ptr<QGraphicsPolygonItem> uiItem;
    std::shared_ptr<QGraphicsPolygonItem> uiItemAvoidance;
};
