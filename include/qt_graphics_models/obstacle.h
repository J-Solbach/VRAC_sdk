#pragma once

#include <QPolygonF>
#include <QGraphicsPolygonItem>
#include <QPen>

namespace vrac::qt_graphics::models {

struct obstacle
{
    obstacle(QPointF center_pos, unsigned int radius, qreal robot_width, const QVector<QPointF>& outer = QVector<QPointF>()) {
        poly = outer;
        // can be reduced to improve calculation time but will reduce precision of the avoidance
        poly.append(generatePolygon(center_pos, radius));

        ui_item = std::make_shared<QGraphicsPolygonItem>(poly);
        ui_item->setFlags(QGraphicsItem::ItemIsMovable);
        ui_item_avoidance = std:: make_shared<QGraphicsPolygonItem>();
        ui_item_avoidance->setParentItem(ui_item.get());

        poly_avoidance = generatePolygon(center_pos, radius + robot_width);
        ui_item_avoidance->setPolygon(poly_avoidance);

        ui_item->setBrush(QBrush(Qt::black));
        ui_item->setOpacity(0.5);
        ui_item_avoidance->setPen(QPen(Qt::green, 20));
    }

    obstacle(QPolygonF poly, QPolygonF poly_avoidance = QPolygonF()) : poly(poly), poly_avoidance(poly_avoidance) {
        ui_item = std::make_shared<QGraphicsPolygonItem>(poly);
        ui_item->setBrush(QBrush(Qt::black));
        ui_item->setPen(QPen(Qt::red, 5));
        ui_item->setOpacity(1);

        if (poly_avoidance.isEmpty()) return;
        ui_item_avoidance = std::make_shared<QGraphicsPolygonItem>(poly_avoidance);
        ui_item_avoidance->setParentItem(ui_item.get());
    }

    static QPolygonF generatePolygon(QPointF center_pos, unsigned int radius) {
        QPolygonF octogon;
        const int vertices = 8;
        for(int i=0; i< vertices; i++)
        {
            QPointF octoPoint;
            octoPoint.setX(center_pos.x() + radius * cos((i + 0.5) * 2 * 3.1415 / vertices));
            octoPoint.setY(center_pos.y() + radius * sin( (i + 0.5) * 2 * 3.1415 / vertices));
            octogon.append(octoPoint);
        }

        return octogon;
    }

    QPolygonF poly;
    QPolygonF poly_avoidance;
    std::shared_ptr<QGraphicsPolygonItem> ui_item;
    std::shared_ptr<QGraphicsPolygonItem> ui_item_avoidance;
};

}

#include <fmt/format.h>
#include <qt_utils/qt_formatters.hpp>

template<>
struct fmt::formatter<vrac::qt_graphics::models::obstacle> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    static auto format(const vrac::qt_graphics::models::obstacle & val, format_context& ctx) {
        return fmt::format_to(ctx.out(), "obstacle polygon ({})", val.poly);
    }
};
