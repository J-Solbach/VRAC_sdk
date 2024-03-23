#pragma once

#include <QPolygonF>
#include <QGraphicsPolygonItem>

#include "path_step.h"
#include "range/v3/all.hpp"

namespace vrac::path_finding::path_checker
{
template<typename obstacles_t>
static bool is_area_free(QPolygonF &area, const obstacles_t & obstacles) {
    QGraphicsPolygonItem area_item(area);
    return std::ranges::any_of(obstacles, [&area_item](const auto & obstacle){ obstacle.ui_item->collidesWithItem(&area_item);});
}

template<typename path_t, typename obstacles_t, typename robot_hitbox_t>
static bool check_path(const path_t & path, const obstacles_t & obstacles, const robot_hitbox_t & hitbox) {
    auto invalid_steps = path
        | ranges::views::filter([&obstacles](const auto & step){
                                return std::ranges::any_of(obstacles, [&step](const auto & obstacle){
                                   return obstacle.ui_item->collidesWithItem(step.ui_item.get());
                                });
        })
        | ranges::views::transform([&hitbox](const auto & step){
                                step.ui_item->setPen(QPen(Qt::red, hitbox.width()));
                                return step;
                                });

    return std::ranges::empty(invalid_steps);
}

template<typename obstacles_t, typename robot_hitbox_t>
static bool check_path(const QPointF& start, const QPointF& goal, const obstacles_t & obstacles, const robot_hitbox_t & hitbox) {
    path_step step(start, goal, hitbox.width());
    step.ui_item->setPen(QPen(Qt::blue, hitbox.width()));

    if (ranges::any_of(obstacles, [&step](const auto & obstacle){
                                return obstacle.ui_item->collidesWithItem(step.ui_item.get());
                            })) {
        return false;
    }

    return true;
}
};
