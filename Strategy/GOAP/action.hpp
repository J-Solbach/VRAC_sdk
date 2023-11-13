#pragma once

#include <string>

template <typename world_state_snapshot_t, typename runner_type, typename cost_calculator_type>
class action
{
public:

    action(const std::string_view & name, world_state_snapshot_t && preconditions, world_state_snapshot_t && effects, runner_type && runner, cost_calculator_type && cost_calculator) :
        name_(name)
        , preconditions_(std::move(preconditions))
        , effects_(std::move(effects))
        , runner_(std::move(runner))
        , cost_calculator_(std::move(cost_calculator)) {
    }

    [[nodiscard]] bool run(const world_state_snapshot_t & ws) const {return std::invoke(runner_, ws);}
    [[nodiscard]] unsigned int cost() const {return std::invoke(cost_calculator_);}
    [[nodiscard]] const std::string_view & name() const {return name_;}
    [[nodiscard]] const world_state_snapshot_t & preconditions() const {return preconditions_;}
    [[nodiscard]] const world_state_snapshot_t & effects() const {return effects_;}

private:
    const std::string_view name_;
    world_state_snapshot_t preconditions_;
    world_state_snapshot_t effects_;
    runner_type runner_;
    cost_calculator_type cost_calculator_;
};