#pragma once

#include <deque>
#include "range/v3/all.hpp"
#include "planner.hpp"

template<typename world_state_type, typename goal_type, typename action_type>
class goap_manager {
    using goal_pool = std::vector<goal_type>;
    using action_pool = std::vector<action_type>;
    using plan_type = std::deque<action_type*>;

public:
    goap_manager(world_state_type && ws, goal_pool && goals, action_pool && actions) :
        world_state(std::move(ws))
        , goals(std::move(goals))
        , actions(std::move(actions)){
    }

    goal_type* select_goal(const world_state_type::world_snapchot & ws) {

        auto valid_goals = goals | ranges::views::filter([&ws](const auto & goal) { return goal.is_valid(ws);}) | ranges::to<std::vector>;
        if (valid_goals.empty()) return nullptr;

        auto it = ranges::max_element(goals, [&ws](const auto & lhs, const auto & rhs){return lhs.priority(ws) > rhs.priority(ws);});
        return &(*it);
    }

    void update() {
        auto snapshot = world_state.get();
        auto new_goal = select_goal(snapshot);

        if (new_goal == nullptr) {
            return;
        }
        else if (new_goal != current_goal) {
            current_goal = new_goal;
            plan = planner::make_plan(*current_goal, actions, snapshot);
        }

        if (plan.empty()) {
            current_goal = nullptr;
            return;
        }

        // need to add a failure component which will trigger the recalculation of a plan.
        if (plan.front()->run(snapshot)) plan.pop_front();
    }

private:
    world_state_type world_state;
    goal_pool goals;
    action_pool actions;

    goal_type *current_goal;
    plan_type plan;
};