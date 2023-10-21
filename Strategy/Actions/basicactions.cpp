#include "basicactions.h"

#include "gamestate.h"
#include "Avoidance/avoidancemanager.h"
#include "Detection/detectionutilities.h"

void Line::onEntry(Event&)
{
    int dist = mParams["dist"].toInt();
    AvoidanceManager::get()->doLine(dist);
}

void Line::onExit(Event&)
{
}

void Rotate::onEntry(Event&)
{
    int theta = mParams["theta"].toInt();
    AvoidanceManager::get()->rotate(theta);
}

void Rotate::onExit(Event&)
{
}

void SetGameState::onEntry(Event &)
{
    QString state = mParams["state"].toMap()["selected"].toString();
    QVariant value = mParams["value"];
    GameState::get()->setState(state, value);
}

void SetGameState::onExit(Event &)
{

}

void TakeGameElement::onEntry(Event &)
{
    QString elementGroup = mParams["group"].toMap()["selected"].toString();
    QString tag = mParams["tag"].toMap()["selected"].toString();

    GameElement *element = [&](){
        if (tag != "") return GameState::get()->playground().popElement(elementGroup, tag);
        else return GameState::get()->playground().popElement(elementGroup, size_t{0});
    }();

    if (element != nullptr) {
     // add to inventory ?
        delete element;
    }
}

void TakeGameElement::onExit(Event &)
{

}

void MoveGameElement::onEntry(Event &)
{
    QString fromGroup = mParams["from"].toMap()["selected"].toString();
    QString toGroup = mParams["to"].toMap()["selected"].toString();
    QString tag = mParams["tag"].toMap()["selected"].toString();

    GameElement *element = [&](){
        if (tag != "") return GameState::get()->playground().popElement(fromGroup, tag);
        else return GameState::get()->playground().popElement(fromGroup, size_t{0});
        }();

    GameState::get()->playground().addElement(toGroup, element);
}

void MoveGameElement::onExit(Event &)
{

}

