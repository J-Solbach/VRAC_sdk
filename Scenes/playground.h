#pragma once

#include <QGraphicsScene>
#include <QObject>
#include <range/v3/all.hpp>

#include "Scenes/obstacle.h"
#include "Scenes/gameelement.h"
#include "Avoidance/pathstep.h"

class Playground : public QGraphicsScene
{
    using element_group_t = std::vector<GameElement*>;
public:
    explicit Playground(QObject *parent = nullptr) : QGraphicsScene(parent) {}
    virtual ~Playground() {clearElements();}

    GameElement* popElement(std::string groupName, GameElement *e) {
        auto & group = getElementGroup(groupName);
        auto itObj = ranges::remove(group, e); // put e at the end of group

        if (itObj == group.end()) return nullptr;
        group.pop_back();
        return *itObj;
    }

    GameElement* popElement(std::string groupName, std::string name)
    {
        auto & group = getElementGroup(groupName);
        auto itObj = ranges::remove_if(group, [&name](auto * item){return item->name() == name;});

        if (itObj == group.end()) return nullptr;
        return *itObj;
    }

    const std::unordered_map<std::string, element_group_t> &elements() const {return mElements;}
    element_group_t & getElementGroup(std::string groupName) {return mElements[groupName];}
    GameElement* getClosestElement(std::string groupName, QPointF currentPos) {
        auto & group = getElementGroup(groupName);

        auto closest = ranges::min_element(group, [&](auto *lhs, auto *rhs) {
            return QLineF(currentPos, lhs->entryPoint()).length() < QLineF(currentPos, rhs->entryPoint()).length();
        });
        return *closest;
    }

public slots:
    void clearElements() {
        auto to_delete = mElements | ranges::views::values | ranges::views::join;
        ranges::for_each(to_delete, [&](auto * element){
            removeItem(element);
            delete element;
        });
        mElements.clear();
    }

    void addElement(std::string groupName, GameElement* element) {
        addItem(element);
        mElements[groupName].push_back(element);
    }

    void removeElement(std::string groupName, std::string name) {
        GameElement* e = popElement(groupName, name);
        removeItem(e);
        delete e;
    }

    void onNewObstacles(const std::vector<Obstacle> &newObstacles)
    {
        for (const auto & obstacle : newObstacles) {
            addItem(obstacle.uiItem.get());
            addItem(obstacle.uiItemAvoidance.get());
        }
    }

    void onNewPath(const std::vector<PathStep>& path) {
        for (const auto & newStep : path)
            addItem(newStep.uiItem.get());
    }

private:
    std::unordered_map<std::string, element_group_t> mElements;
};
