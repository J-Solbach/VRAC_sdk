#ifndef PATH_PLANNER_H
#define PATH_PLANNER_H

#include <QPointF>
#include <QVector>

#include "pathstep.h"
#include "path_checker.h"

#include <range/v3/all.hpp>

typedef struct vertex
{
    vertex(QPointF p, qreal d = 0xFFFF){ pos = p, distance = d, visited = false; previous = nullptr;}
    QPointF pos;
    qreal distance;
    bool visited;

    QVector<std::shared_ptr<vertex>>neighbors;
    std::shared_ptr<vertex> previous;
} vertex;

namespace path_planner
{

template<typename vertices_t>
static std::shared_ptr<vertex> nearest_non_visited_vertex(vertices_t && vertices) {

    auto non_visited_vertices = (vertices
            | ranges::views::filter([](const auto & vertex){
                                       return !vertex->visited;
                                   }));

    auto min_element = ranges::min_element(non_visited_vertices, {}, [](const auto & vertex) {return vertex->distance;});

    if (min_element != std::end(non_visited_vertices))
        return *min_element;
    return
        nullptr;
}

template<typename vertices_t>
static std::shared_ptr<vertex> getShortestPathRewinder(vertices_t && vertices, const QPointF& goal) {
    std::shared_ptr<vertex> rewinder = nullptr;

    //REFACTO THIS
    for (int ind = 0; ind < vertices.size(); ind++)
    {
        //take the non visited vertice with the minimum dist in the graph
        std::shared_ptr<vertex> currentVertex = nearest_non_visited_vertex(vertices);

        if (currentVertex == nullptr) continue;

        currentVertex->visited = true;

        auto goalExist = [&](std::shared_ptr<vertex> neighbor)
        {
            if (neighbor == nullptr) return false;
            qreal totalDist = currentVertex->distance + QLineF(currentVertex->pos, neighbor->pos).length();
            // If it's shorter !
            if (totalDist < neighbor->distance && neighbor->visited == false)
            {
                neighbor->distance = totalDist;
                neighbor->previous = currentVertex;
            }
            return neighbor->pos == goal;
        };

        auto ret = std::find_if(currentVertex->neighbors.begin(), currentVertex->neighbors.end(), goalExist);

        if (ret != currentVertex->neighbors.end())
        {
            if (rewinder == nullptr) rewinder = *ret;
            else if ((*ret).get()->distance < rewinder.get()->distance)
            {
                rewinder = *ret;
            }
        }
    }

    return rewinder;
}


template<typename obstacles_t, typename hitbox_t>
static void setupNeighbors(QVector<std::shared_ptr<vertex>>& vertices, obstacles_t && obstacles, hitbox_t && hitbox) {

    auto filtered_vertices = vertices | ranges::views::filter([](const auto & vertex) {return vertex != nullptr;});

    ranges::for_each(filtered_vertices, [&](auto & vertex){
        vertex->neighbors = filtered_vertices
                            | ranges::views::filter([&](const auto &v) {
                                    return v->pos != vertex->pos &&
                                    path_checker::checkPath(vertex->pos, v->pos, obstacles, hitbox);
                                })
                            | ranges::to<QVector>;
    });
}

template<typename obstacles_t>
static QVector<std::shared_ptr<vertex>> gatherVertices(const QPointF& start, const QPointF& goal, obstacles_t && obstacles) {
    QVector<std::shared_ptr<vertex>> vertices;
    vertices <<  std::make_shared<vertex>(vertex(start, 0));

    vertices << (obstacles
                    | ranges::views::transform([](const auto & obstacle){
                          return obstacle.uiItemAvoidance->mapToScene(obstacle.polyAvoidance);
                    })
                    | ranges::views::join // all vertex of a the obstacle polygon
                    | ranges::views::transform([](const auto & vertice) {
                        return std::make_shared<vertex>(vertex(vertice, 0xFFFF)); //0xFFFF for max dist
                    })
                    |ranges::to<QVector>);

    vertices << std::make_shared<vertex>(vertex(goal, 0xFFFF)); //0xFFFF for max dist
    return vertices;
}

template<typename obstacles_t, typename hitbox_t>
static QVector<PathStep> findPath(const QPointF& start, const QPointF& goal, obstacles_t && obstacles, hitbox_t && hitbox) {
    auto vertices = gatherVertices(start, goal, obstacles);
    setupNeighbors(vertices, obstacles, hitbox);
    std::shared_ptr<vertex> rewinder = getShortestPathRewinder(vertices, goal);

    // No path available, resetting to straight line to goal, for the moment
    if ( rewinder == nullptr)
    {
        return QVector<PathStep>();
    }

    QVector<PathStep> newPath;

    //BackTracking the path from the rewinder
    while (rewinder->previous != nullptr)
    {
        newPath.emplaceFront(rewinder->previous->pos, rewinder->pos); // straight line
        rewinder = rewinder->previous;
    }

    return newPath;
};

template<typename obstacles_t, typename hitbox_t>
static QVector<PathStep> findPath(const PathStep& path_step, obstacles_t && obstacles, hitbox_t && hitbox) {
    return findPath(path_step.start, path_step.goal, obstacles, hitbox);
}

template<typename obstacles_t, typename hitbox_t>
static QVector<PathStep> findPath(const QVector<PathStep> &path, obstacles_t && obstacles, hitbox_t && hitbox) {
    if (path.isEmpty()) return QVector<PathStep>();

    return findPath(path.front().start, path.back().goal, obstacles, hitbox);
}

}

#endif // PATH_PLANNER_H
