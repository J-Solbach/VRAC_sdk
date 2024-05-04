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
        QList<QGraphicsItem*> all = items();
        for (int i = 0; i < all.size(); i++)
        {
            QGraphicsItem *gi = all[i];
            removeItem(gi);
        }
    }

public slots:

    void addElement(game_element* element) {
        addItem(element);
    }

    void on_new_obstacles(const std::vector<obstacle> &newObstacles) {
        for (const auto & obs : newObstacles)
            addItem(obs.ui_item.get());
    }

    void on_new_path(const std::vector<path_finding::path_step>& path) {
        for (const auto & newStep : path) {
            addItem(newStep.ui_item.get());
        }
    }
};
}
