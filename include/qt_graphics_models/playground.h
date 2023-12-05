#pragma once

#include <QGraphicsScene>
#include <QObject>
#include <range/v3/all.hpp>

#include "qt_graphics_models/obstacle.h"
#include "qt_graphics_models/game_element.h"
#include "path_finding/path_step.h"

class Playground : public QGraphicsScene
{
public:
    explicit Playground(QObject *parent = nullptr) : QGraphicsScene(parent) {}
    virtual ~Playground() {
    }

public slots:

    void addElement(GameElement* element) {
        addItem(element);
    }

    void onNewObstacles(const std::vector<Obstacle> &newObstacles) {
        for (const auto & obstacle : newObstacles) {
            addItem(obstacle.uiItem.get());

            if (obstacle.uiItemAvoidance.get() != nullptr)
                addItem(obstacle.uiItemAvoidance.get());
        }
    }

    void onNewPath(const std::vector<PathStep>& path) {
        for (const auto & newStep : path)
            addItem(newStep.uiItem.get());
    }
};
