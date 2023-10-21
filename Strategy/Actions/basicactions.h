#ifndef BASICACTIONS_H
#define BASICACTIONS_H

#include "Strategy/STM/state.h"

class Line : public State {
public:
    Line(QString tag, QVariantMap params) : State(tag, params) {}

    virtual void onEntry(Event&) override;
    virtual void onExit(Event&) override;
};

class Rotate : public State {
public:
    Rotate(QString tag, QVariantMap params) : State(tag, params) {}

    virtual void onEntry(Event&) override;
    virtual void onExit(Event&) override;
};

class MoveTo : public State {
public:
    MoveTo(QString tag, QVariantMap params) : State(tag, params) {}

    virtual void onEntry(Event&) override;
    virtual void onExit(Event&) override;
};

class GoToClosestGameElement : public State {
public:
    GoToClosestGameElement(QString tag, QVariantMap params) : State(tag, params) {}

    virtual void onEntry(Event&) override;
    virtual void onExit(Event&) override;
};

class SetGameState : public State {
public:
    SetGameState(QString tag, QVariantMap params) : State(tag, params) {}

    virtual void onEntry(Event&) override;
    virtual void onExit(Event&) override;
};

class TakeGameElement : public State {
public:
    TakeGameElement(QString tag, QVariantMap params) : State(tag, params) {}

    virtual void onEntry(Event&) override;
    virtual void onExit(Event&) override;
};

class MoveGameElement : public State {
public:
    MoveGameElement(QString tag, QVariantMap params) : State(tag, params) {}

    virtual void onEntry(Event&) override;
    virtual void onExit(Event&) override;
};


#endif // BASICACTIONS_H
