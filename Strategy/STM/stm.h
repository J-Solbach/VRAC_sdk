#ifndef STM_H
#define STM_H

#include "state.h"
#include "transition.h"

#include <range/v3/all.hpp>
#include <QObject>
#include <set>

template<typename state_t>
class Stm : public state_t
{

public:
    Stm(std::string name) : state_t(name){ connect(this, &state_t::sendEvent, this, &state_t::onEvent, Qt::QueuedConnection); }

    ~Stm()
    {
        for (auto * state : mStates | ranges::views::values) {
            delete state;
        }
    }

    void update() {
        Event e{"NoEvent"};
        onEntry(e);
        onEvent(e);
    }

    virtual void onEntry(state_t::event_t & e) override  {
        if (mInitialState == nullptr) {
            finished();
            return;
        }

        mCurrentStates.clear();
        mCurrentStates.push_back(mInitialState);
        mInitialState->onEntry(e);
    }

    void addState(std::string tag, state_t * s)
    {
        connect(s, &state_t::sendEvent, this, &state_t::onEvent, Qt::QueuedConnection);
        mStates[tag] = s;
    }

    const auto &states() const { return mStates; }
    void setStates(std::unordered_map<std::string, state_t *> &&newStates) { mStates = std::move(newStates); }

    std::vector<state_t *> &currentStates() { return mCurrentStates; }
    void setCurrentStates(const std::vector<state_t *> &newCurrentStates) { mCurrentStates = newCurrentStates; }
    void setInitialState(state_t *s) { mInitialState = s; }

signals:
    void finished();

public slots:
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

        ranges::for_each(to_be_removed, [&](const auto state){
            state->onExit(e);
            ranges::remove_if(mCurrentStates, [&state](auto * s) {return state == s;});
        });

        ranges::for_each(to_be_added, [&](const auto state){
            state->onEntry(e);
        });

        mCurrentStates.insert(std::end(mCurrentStates), std::begin(to_be_added), std::end(to_be_added) );
        state_t::sendEvent(Event{"NoEvent"});
        return std::vector<std::string>{};
    }

private:
    std::unordered_map<std::string, state_t*> mStates;
    std::vector<state_t*> mCurrentStates;
    state_t* mInitialState;
};

#endif // STM_H
