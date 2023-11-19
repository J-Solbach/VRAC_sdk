#ifndef __STATE_H
#define __STATE_H

#include <QObject>
#include <QVariant>
#include <set>
#include <functional>
#include <range/v3/all.hpp>

#include "event.h"
#include "transition.h"

class state_signals_slots :  public QObject {
    Q_OBJECT

signals :
    void sendEvent(Event e);

public slots:
    virtual std::vector<std::string> onEvent(Event e) = 0;
};

template<typename context_t, typename params_t>
class State : state_signals_slots
{
public:
    State(std::string name) : state_signals_slots(), mCheckSum(std::hash<std::string>{}(name)) {
       ;
        setObjectName("[" + name + "]");
        mCheckSum += (rand() %0xFFFF);
    }

    State(const std::string & name,  const params_t & params) : state_signals_slots(), mParams(params), mCheckSum(std::hash<std::string>{}(name)) {
        setObjectName("[" + name + "]");
        mCheckSum += (rand() %0xFFFF);
    }

    void addTransition(transition && t) { mTransitions.push_back(std::move(t)); }

    const std::vector<transition> &transitions() const {
        return mTransitions;
    }
    void setTransitions(std::vector<transition> && newTransitions) {
        mTransitions = std::move(newTransitions);
    }

    bool testCheckSum(uint16_t checkSum) {
        if (checkSum == 0xFFFF) return true;

        if (checkSum != mCheckSum ) return false;

        if (mCheckCounter > 0) mCheckCounter--;

        return (mCheckCounter == 0);
    }

    uint16_t checkSum() {
        if (mCheckCounter == 0xFF) mCheckCounter = 0;

        mCheckCounter++;
        return mCheckSum;
    }

    virtual std::vector<std::string> onEvent(Event e) {

        return mTransitions
                | ranges::views::filter([&e](const auto & transition){
                    return transition.event == e;
                    })
                | ranges::views::transform([](const auto & transition) {
                     return transition.target_state;
                    })
                | ranges::to<std::vector>;
    }

    virtual void onEntry(Event&) = 0;
    virtual void onExit(Event&) {}

protected:
    std::vector<transition> mTransitions;
    params_t mParams;

    std::size_t mCheckSum = 0xFFFF;
    uint16_t mCheckCounter = 0xFFFF;
};

#endif
