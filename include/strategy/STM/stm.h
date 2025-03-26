#pragma once

#include "event.h"
#include "state.h"

#include <spdlog/spdlog.h>
#include <fmt/color.h>
#include <fmt/ranges.h>
#include <range/v3/all.hpp>
#include <QObject>
#include <set>

namespace vrac::strategy::state_machines {

template<typename context_type, typename params_type>
class Stm : public state<context_type, params_type>
{

    using state_t = state<context_type, params_type>;

public:

    Stm(std::string name, context_type & ctx, state_t* initial_state, std::unordered_map<std::string, state_t*> && states) :
        state_t(name),
        states(states),
        entry_state(initial_state),
        ctx(ctx)
    {
        for (const auto & state : states) {
            state_t * s = state.second;
            QObject::connect(s, &state_t::send_event, this, &Stm::on_event, Qt::QueuedConnection);
        }
    }

    ~Stm() {
        for (auto * state : states | ranges::views::values) {
            delete state;
        }
    }

    void start() {
        event e{"NoEvent"};
        on_entry(ctx,e);
    }

    void update() {}

    virtual void on_entry(context_type &, event  e) override  {
        if (entry_state == nullptr) {
            return;
        }

        current_states.clear();
        current_states.push_back(entry_state);

        spdlog::info("Entering state machine {} -> {}", state_t::name, *entry_state);
        entry_state->on_entry(ctx,e);
        auto noevent = vrac::strategy::state_machines::event{"NoEvent"};
        on_event(noevent);
    }

    virtual std::vector<std::string> on_event(event & e) override
    {
        spdlog::info("{}({}) notified with event : {}", state_t::name, fmt::join(current_states, ","), e.value);

        std::unordered_map<state_t*, std::vector<std::string>> state_transition_map;
        for (auto * state : current_states) {
            auto target_states = state->on_event(e);

            if (!std::empty(target_states)) {
                state_transition_map[state] = target_states;
            }
        }

        // no transitions in sub states. checking as a state
        if (std::empty(state_transition_map) && !e.consumed) {
            return state_t::on_event(e);
        }

        e.consumed = true;

        auto to_be_removed = state_transition_map
                             | ranges::views::keys
                             | ranges::to<std::vector>;

        auto to_be_added = state_transition_map
                           | ranges::views::values
                           | ranges::views::join
                           | ranges::views::transform([&](const auto & state_tag){
                                 return states.at(state_tag);
                             })
                           | ranges::to<std::vector>;

        spdlog::info("{} {} {}",
               fmt::format("{}", fmt::join(to_be_removed, "")),
               fmt::format("--[{}]-->", e.value),
               fmt::format("{}", fmt::join(to_be_added, ""))
        );

        ranges::for_each(to_be_removed, [&](auto *state){
            state->on_exit(ctx, e);
            ranges::erase(current_states, ranges::remove(current_states, state), std::end(current_states));
        });

        // state without transitions should just be called and not inserted.
        ranges::for_each(to_be_added, [&](auto * state){
            if (!std::empty(state->get_transitions())
            && ranges::find_if(current_states, [&](auto *cs){
                return cs->get_name() == state->get_name();
            }) == std::end(current_states)) return;

            state->on_entry(ctx, e);
            ranges::erase(to_be_added, ranges::remove(to_be_added, state), std::end(to_be_added));
        });

        current_states.insert(std::end(current_states), std::begin(to_be_added), std::end(to_be_added) );
        ranges::for_each(to_be_added, [&](auto * state){
            state->on_entry(ctx, e);
        });

        auto noevent = vrac::strategy::state_machines::event{"NoEvent"};
        on_event(noevent);
        return std::vector<std::string>{};
    }

    const auto &get_states() const { return states; }
    std::vector<state_t *> &get_current_states() { return current_states; }

private:
    std::unordered_map<std::string, state_t*> states;
    std::vector<state_t*> current_states;
    state_t* entry_state;
    context_type & ctx;
};

}
