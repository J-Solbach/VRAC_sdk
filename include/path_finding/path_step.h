#pragma once

#include <QPointF>
#include <QPen>
#include <QGraphicsPathItem>

namespace vrac::path_finding::details {
    static QPointF calculate_step(std::size_t step, QPointF s, QPointF c1, QPointF c2, QPointF g) {
        double t = (1.0 / 100) * step;
        // Calculate the cubic Bezier curve coordinates
        double t_squared = t * t;
        double t_cubed = t_squared * t;

        double one_minus_t = 1.0 - t;
        double one_minus_t_squared = one_minus_t * one_minus_t;
        double one_minus_t_cubed = one_minus_t_squared * one_minus_t;

        double start_x = s.x();
        double start_y = s.y();
        double check_x = c1.x();
        double check_y = c1.y();
        double check2_x = c2.x();
        double check2_y = c2.y();
        double end_x = g.x();
        double end_y = g.y();

        double x_target = one_minus_t_cubed * start_x + 3 * one_minus_t_squared * t * check_x +
                          3 * one_minus_t * t_squared * check2_x + t_cubed * end_x;
        double y_target = one_minus_t_cubed * start_y + 3 * one_minus_t_squared * t * check_y +
                          3 * one_minus_t * t_squared * check2_y + t_cubed * end_y;

        return QPointF(x_target, y_target);
    }

    static std::vector<QPointF> make_path_waypoints(QPointF s,  QPointF c1, QPointF c2, QPointF g) {
        return ranges::views::iota(0)
            | ranges::views::take(100)
            | ranges::views::transform([&](auto i){
                return calculate_step(i, s, c1, c2, g);
            })
            | ranges::to<std::vector>();
    }

    static QPainterPath make_path(const auto & points) {

        QPainterPath ret;
        ret.moveTo(points.front());
        ranges::for_each(points, [&](auto point){
            ret.lineTo(point);
        });

        return ret;
    }
}

namespace vrac::path_finding {
struct path_step
{
    using ui_item_t = std::shared_ptr<QGraphicsPathItem>;
    path_step(const QPointF s, const QPointF c1, const QPointF c2, const QPointF g, qreal width) : start(s), check_point_1(c1), check_point_2(c2), goal(g) {

        waypoints = details::make_path_waypoints(start, check_point_1, check_point_2, goal);
        ui_item = std::make_shared<QGraphicsPathItem>(details::make_path(waypoints));

        QPen pen(Qt::blue, width);
        ui_item->setPen(pen);
        ui_item->setOpacity(0.3);
    }

    path_step(QPointF s, QPointF g, qreal width) : start(s), check_point_1(g), check_point_2(g), goal(g) {
        waypoints = details::make_path_waypoints(start, check_point_1, check_point_2, goal);
        ui_item = std::make_shared<QGraphicsPathItem>(details::make_path(waypoints));


        QPen pen(Qt::blue, width);
        ui_item->setPen(pen);
        ui_item->setOpacity(0.3);
    }

    void setStart(const QPointF s)
    {
        auto nearest = ranges::min_element(waypoints, [&](const auto & p1, const auto & p2){
                return QLineF(s, p1).length() < QLineF(s, p2).length();
        });

        std::size_t step = std::distance(std::begin(waypoints), nearest);
        ui_item->setPath(details::make_path(waypoints | ranges::views::drop(step)));
    }

    QPointF start;
    QPointF check_point_1;
    QPointF check_point_2;
    QPointF goal;

    std::shared_ptr<QGraphicsPathItem> ui_item;
    std::vector<QPointF> waypoints;
};
}

#include <fmt/core.h>
#include <fmt/format.h>
#include <qt_utils/qt_formatters.hpp>

template<>
struct fmt::formatter<vrac::path_finding::path_step> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    static auto format(const vrac::path_finding::path_step & val, format_context& ctx) {
        return fmt::format_to(ctx.out(), "{} -> {}", val.start, val.goal);
    }
};
