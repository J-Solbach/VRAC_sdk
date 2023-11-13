#pragma once

#include <string>

template <typename world_state_snapshot_t, typename priority_calculator_type>
class goal
{
public:

    goal(world_state_snapshot_t && desired_world_state, world_state_snapshot_t && preconditions, priority_calculator_type && priority_calculator) :
        name_("name")
        , desired_world_state_(std::move(desired_world_state))
        , preconditions_(std::move(preconditions))
        , priority_calculator_(std::move(priority_calculator)) {
    }

    [[nodiscard]] bool is_valid(const world_state_snapshot_t & ws) const {
        for (const auto & [key, value] : preconditions_) {
            if (ws.at(key) != value) return false;
        }
        return true;
    }

    [[nodiscard]] const std::string_view & name() const {return name_;}
    [[nodiscard]] const world_state_snapshot_t & preconditions() const {return preconditions_;}
    [[nodiscard]] const world_state_snapshot_t & desired_world_state() const {return desired_world_state_;}
    [[nodiscard]] unsigned int priority(const world_state_snapshot_t & ws) const{return std::invoke(priority_calculator_, ws);}

private:
    const std::string_view name_;
    world_state_snapshot_t desired_world_state_;
    world_state_snapshot_t preconditions_;
    priority_calculator_type priority_calculator_;
};