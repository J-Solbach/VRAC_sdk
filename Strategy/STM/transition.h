#ifndef TRANSITION_H
#define TRANSITION_H

#include <QString>
#include <QObject>
#include <set>

#include "Strategy/STM/event.h"

struct transition {
    std::string target_state;
    Event event;
};

constexpr bool event_test(const transition & transition, const Event & e) {
    return (transition.event == e);
}

#endif // TRANSITION_H
