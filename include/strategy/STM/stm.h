#pragma once

#include "event.h"

#include <fmtlog/fmtlog.h>
#include <range/v3/all.hpp>
#include <QObject>
#include <set>


// TODO RAII Stm
template<typename state_t>
class Stm : public state_t
{

public:
    Stm(std::string name, state_t::context_type & ctx) : state_t(name), ctx(ctx){
    }

    Stm(std::string name, state_t::context_type & ctx, std::vector<state_t> states) : state_t(name), mStates(states), ctx(ctx) {
    }

    ~Stm()
    {
        for (auto * state : mStates | ranges::views::values) {
            delete state;
        }
    }

    void start() {
        Event e{"NoEvent"};
        onEntry(ctx,e);
        onEvent(e);
    }

    void update() {
    }

    void addState(std::string tag, state_t * s) {
        connect(s, &state_t::sendEvent, this, &state_t::onEvent, Qt::QueuedConnection);
        mStates[tag] = s;
    }

    const auto &states() const { return mStates; }
    void setStates(std::unordered_map<std::string, state_t *> && newStates) { mStates = std::move(newStates); }

    std::vector<state_t *> &currentStates() { return mCurrentStates; }
    void setCurrentStates(const std::vector<state_t *> &newCurrentStates) { mCurrentStates = newCurrentStates; }
    void setInitialState(state_t *s) { mInitialState = s; }

    virtual void onEntry(state_t::context_type &, Event  e) override  {
        if (mInitialState == nullptr) {
            return;
        }

        mCurrentStates.clear();
        mCurrentStates.push_back(mInitialState);

        FMTLOG(fmtlog::INF, "{} on Entry", mInitialState);
        mInitialState->onEntry(ctx,e);
        state_t::sendEvent(Event{"NoEvent"});
    }

    virtual std::vector<std::string> onEvent(Event e) override
    {
        const auto state_transition_map = mCurrentStates
                                          | ranges::views::transform([&](auto * state){
                                                return std::pair{state, state->onEvent(e)};
                                            })
                                          | ranges::views::filter([](const auto & entry) {
                                                auto [_, target_states] = entry;
                                                return !std::empty(target_states);
                                            })
                                          | ranges::to<std::unordered_map<state_t*, std::vector<std::string>>>;

        // no transitions in sub states. checking as a state
        if (std::empty(state_transition_map)) {
            return state_t::onEvent(e);
        }

        auto to_be_removed = state_transition_map
                             | ranges::views::keys
                             | ranges::to<std::vector>;

        auto to_be_added = state_transition_map
                           | ranges::views::values
                           | ranges::views::join
                           | ranges::views::transform([&](const auto & state_tag){
                                 return mStates.at(state_tag);
                             })
                           | ranges::to<std::vector>;

        ranges::for_each(to_be_removed, [&](auto *state){
            FMTLOG(fmtlog::INF, "{} on Exit", state);
            state->onExit(ctx, e);
            ranges::erase(mCurrentStates, ranges::remove(mCurrentStates, state), std::end(mCurrentStates));
        });

        ranges::for_each(to_be_added, [&](auto * state){
            FMTLOG(fmtlog::INF, "{} on Entry", state);
            state->onEntry(ctx, e);
        });

        mCurrentStates.insert(std::end(mCurrentStates), std::begin(to_be_added), std::end(to_be_added) );
        state_t::sendEvent(Event{"NoEvent"});
        return std::vector<std::string>{};
    }

private:
    std::unordered_map<std::string, state_t*> mStates;
    std::vector<state_t*> mCurrentStates;
    state_t* mInitialState;
    state_t::context_type & ctx;
};
