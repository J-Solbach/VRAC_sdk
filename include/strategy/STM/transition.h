#pragma once

#include <string>
#include "strategy/STM/event.h"

namespace vrac::strategy::state_machines {

struct transition {
    std::string target_state;
    vrac::strategy::state_machines::event event;
};

constexpr bool event_test(const transition & transition, const vrac::strategy::state_machines::event & e) {
    return (transition.event == e);
}
}
