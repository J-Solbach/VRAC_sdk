#pragma once

#include <QString>
#include <QObject>
#include <set>

#include "strategy/STM/event.h"

struct transition {
    std::string target_state;
    Event event;
};

constexpr bool event_test(const transition & transition, const Event & e) {
    return (transition.event == e);
}
