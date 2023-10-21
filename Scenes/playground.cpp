#include "playground.h"

#include "gamestate.h"

Playground::Playground(QObject *parent) : QGraphicsScene(parent)
{

}

Playground::~Playground()
{
    clearElements();
}

void Playground::clearElements()
{
     for (auto group : mElements) {
         for (auto element : group) {
             removeItem(element);
             delete element;
         }
     }
     mElements.clear();
}

void Playground::addElement(QString groupName, GameElement* element)
{
    addItem(element);
    mElements[groupName].push_back(element);
}

void Playground::removeElement(QString groupName, QString name)
{
    GameElement* e = popElement(groupName, name);

    removeItem(e);
    delete e;
}

GameElement *Playground::popElement(QString groupName, GameElement *e)
{
    auto & group = getElementGroup(groupName);
    return group.takeAt(group.indexOf(e));
}

GameElement *Playground::popElement(QString groupName, QString name)
{
    auto & group = getElementGroup(groupName);

    auto itObj = std::find_if(
      group.begin(), group.end(),
      [&](GameElement *e) { return e->name() == name; }
    );

    if (itObj == group.end()) return nullptr;

    group.removeOne(*itObj);

    return *itObj;
}

GameElement *Playground::popElement(QString groupName, size_t index)
{
    auto & group = getElementGroup(groupName);
    return group.takeAt(index);
}

QVector<GameElement *> & Playground::getElementGroup(QString groupName)
{
    return mElements[groupName];
}

GameElement *Playground::getClosestElement(QString groupName, QPointF currentPos)
{
    unsigned int closestDist = 0xFFFFFFFF;

    GameElement *closest;

    for (const auto& element : getElementGroup(groupName))
    {
        unsigned int dist = QLineF(currentPos, element->entryPoint()).length();

        if (dist < closestDist)
        {
            closest = element;
            closestDist = dist;
        }
    }
    return closest;
}

void Playground::setObstacles(const QVector<Obstacle> &newObstacles)
{
    for (auto obstacle : mObstacles) {
        removeItem(obstacle.uiItem.get());
    }
    for (auto obstacle : newObstacles) {
        addItem(obstacle.uiItem.get());
    }

    mObstacles = newObstacles;
}

void Playground::setStaticObstacles(const QVector<Obstacle> &newStaticObstacles)
{
    for (auto obstacle : mStaticObstacles) {
        removeItem(obstacle.uiItem.get());
    }
    for (auto obstacle : newStaticObstacles) {
        addItem(obstacle.uiItem.get());
    }
    mStaticObstacles = newStaticObstacles;
}

void Playground::setPath(const QVector<PathStep> &newPath)
{
    for (auto newStep : newPath) addItem(newStep.uiItem.get());

    mPath = newPath;
}

const QVector<PathStep> &Playground::path()
{
    return mPath;
}

void Playground::popPathStep()
{
    mPath.pop_front();
}

void Playground::updatePath()
{
    if (mPath.isEmpty()) return;
    mPath.front().setStart(GameState::get()->robot().pos());
}
