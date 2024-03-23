#pragma once

#include <unordered_map>
#include <functional>

namespace vrac::strategy::goap {

template <typename state_name_t, typename state_type_t>
class world_state
{
public:
    using state_observer = std::function<state_type_t()>;
    using world_snapchot = std::unordered_map<state_name_t, state_type_t>;
    using world_state_observers = std::unordered_map<state_name_t, state_observer>;

    void add_state_observer(const state_name_t &name, const state_observer && obs) {
        observers[name] = obs;
    }

    const state_type_t get_state(const state_name_t & name) {
        return std::invoke(observers.at(name));
    }

    world_snapchot get() {
        world_snapchot ws;
        std::transform(observers.begin(), observers.end(), std::inserter(ws, ws.end()), [](const auto & entry){
            auto [key, func] = entry;
            return std::pair{key, std::invoke(func)};
        });

        return ws;
    }

private:
    world_state_observers observers;
};

}
