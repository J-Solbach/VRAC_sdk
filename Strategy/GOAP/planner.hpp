#pragma once

#include <optional>
#include <deque>
#include <iostream>
#include <ranges>
#include <numeric>

namespace planner::details {

    template <typename Node, typename action_pool_type, typename world_state_snapshot_t>
    [[nodiscard]] bool build_plans(Node & step, action_pool_type & action_pool, const world_state_snapshot_t & current_world_state) {

        bool has_follow_up = false;

        auto desired_state = step.ws;

        // remove states that the world is already currently in.
        std::erase_if(desired_state, [&current_world_state](const auto & entry){
                const auto & [key, value] = entry;
                return current_world_state.contains(key) && current_world_state.at(key) == value;
            });

        // if nothing more desired : Path to GOAL found !
        if (desired_state.empty())
        {
            // A PATH HAS BEEN FOUND for this branch. Stopping here.
            return true;
        }

        for (auto& action : action_pool)
        {
            bool should_use_action = false;
            const world_state_snapshot_t& effects = action.effects();
            world_state_snapshot_t desired_state_with_action = desired_state;

            //checking if an action effect corresponds to the desired world state
            if (std::erase_if(desired_state_with_action, [&effects](const auto & entry){
                const auto & [key, value] = entry;
                return effects.contains(key) && effects.at(key) == value;
            })) {
                should_use_action = true;
            }

            if (!should_use_action) continue;

            // adding new desired states due to the action preconditions
            desired_state_with_action.insert(std::begin(action.preconditions()), std::end(action.preconditions()));

            Node newStep = {
                .action = &action,
                .ws = desired_state_with_action
            };

            if (desired_state_with_action.empty() || build_plans(newStep, action_pool, current_world_state))
            {
                step.childrens.push_back(std::move(newStep));
                has_follow_up = true;
            }
        }
        return has_follow_up;
    }

    template <typename plan_type, typename Node>
    [[nodiscard]] static std::vector<plan_type> gather_all_plans(const Node& node) {

        if (node.childrens.empty() && node.action != nullptr)
        {
            return std::vector<plan_type>{plan_type{node.action}};
        }

        return node.childrens;
//            | std::ranges::views::transform([](const auto & children){return gather_all_plans<plan_type>(children);})
//            | std::ranges::views::join
//            | std::ranges::views::transform([&node](const auto & value){
//                plan_type childPlan = value;
//                if (node.action != nullptr) childPlan.push_back(node.action);
//                return childPlan;
//            })
//               | std::ranges::to<std::vector>;

    }


    template <typename plan_type>
    [[nodiscard]] static auto find_cheapest_plan(const std::vector<plan_type> & plans) {

        if (plans.empty()) return plan_type{};

        auto pairs = plans
                     | std::ranges::views::transform([](const auto & plan){
                           auto total_cost = std::accumulate(std::cbegin(plan), std::cend(plan), 0, [](unsigned int result, const auto & action){ return result += action->cost();});
                        return std::pair{plan, total_cost};
                    });

        return std::get<0>(*std::ranges::min_element(pairs, [](const auto & lhs, const auto & rhs){return lhs.second < rhs.second; }));
    }
}

namespace planner {

    template <typename action_type, typename world_state_snapshot_t>
    struct Node {
        action_type* action;
        world_state_snapshot_t ws;
        std::vector<Node> childrens{};
    };

    template <typename goal_type, typename action_type, typename world_state_snapshot_t>
    [[nodiscard]] static auto make_plan(const goal_type & goal, std::vector<action_type> & action_pool, const world_state_snapshot_t & ws = {}) {
        using plan_type = std::deque<action_type*>;

        world_state_snapshot_t desiredState = goal.desired_world_state();

        if (desiredState.empty()) return plan_type{};

        Node<action_type, world_state_snapshot_t> rootNode {
            .action = nullptr,
            .ws = desiredState
        };

        if (!details::build_plans(rootNode, action_pool, ws)) return plan_type{};

        auto plans = details::gather_all_plans<plan_type>(rootNode);
        return details::find_cheapest_plan(plans);
    }
}
