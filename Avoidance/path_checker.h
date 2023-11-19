#pragma once

#include <QPolygonF>
#include <QGraphicsPolygonItem>
#include <QGraphicsPathItem>

#include "pathstep.h"
#include "range/v3/all.hpp"


namespace path_checker
{
template<typename obstacles_t>
static bool isAreaFree(QPolygonF &area, const obstacles_t && obstacles) {
    QGraphicsPolygonItem areaItem(area);
    return std::ranges::any_of(obstacles, [&areaItem](const auto & obstacle){ obstacle.uiItem->collidesWithItem(&areaItem);});
}

template<typename path_t, typename obstacles_t, typename robot_hitbox_t>
static bool checkPath(path_t && path, obstacles_t && obstacles, robot_hitbox_t && hitbox) {
    auto invalid_steps = path
        | ranges::views::filter([&obstacles](const auto & step){
                                return std::ranges::any_of(obstacles, [&step](const auto & obstacle){
                                   return obstacle.uiItem->collidesWithItem(step.uiItem.get());
                                });
        })
        | ranges::views::transform([&hitbox](const auto & step){
                                step.uiItem->setPen(QPen(Qt::red, hitbox.width()));
                                return step;
                                });

    return std::ranges::empty(invalid_steps);
}

template<typename obstacles_t, typename robot_hitbox_t>
static bool checkPath(const QPointF& start, const QPointF& goal, obstacles_t && obstacles, robot_hitbox_t && hitbox) {
    PathStep step(start, goal, hitbox.width());
    step.uiItem->setPen(QPen(Qt::blue, hitbox.width()));

    if (ranges::any_of(obstacles, [&step](const auto & obstacle){
                                return obstacle.uiItem->collidesWithItem(step.uiItem.get());
                            })) {
        return false;
    }

    return true;
}
};
