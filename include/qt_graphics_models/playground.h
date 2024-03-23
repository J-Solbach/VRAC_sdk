#pragma once

#include <QGraphicsScene>
#include <QObject>
#include <range/v3/all.hpp>

#include "qt_graphics_models/obstacle.h"
#include "qt_graphics_models/game_element.h"
#include "path_finding/path_step.h"

namespace vrac::qt_graphics::models {

class playground : public QGraphicsScene
{
public:
    explicit playground(QObject *parent = nullptr) : QGraphicsScene(parent) {}
    virtual ~playground() {
    }

public slots:
    
    void addElement(game_element* element) {
        addItem(element);
    }
    
    void on_new_obstacles(const std::vector<obstacle> &newObstacles) {
        for (const auto & obstacle : newObstacles) {
            addItem(obstacle.ui_item.get());

            if (obstacle.ui_item_avoidance.get() != nullptr)
                addItem(obstacle.ui_item_avoidance.get());
        }
    }

    void on_new_path(const std::vector<path_finding::path_step>& path) {
        for (const auto & newStep : path)
            addItem(newStep.ui_item.get());
    }
};
}
