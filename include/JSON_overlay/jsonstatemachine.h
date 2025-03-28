#pragma once

#include <fstream>
#include <nlohmann/json.hpp>
#include "strategy/STM/state.h"
#include "strategy/STM/stm.h"
#include "strategy/STM/transition.h"

namespace vrac::json_overlay {

using namespace strategy::state_machines;

template<typename context_type>
struct action_factory{
    using action_t = state<context_type, nlohmann::json>;
    using factory_type = std::function<action_t *(std::string, typename action_t::params_type)>;
    using meta_factory_type = std::unordered_map<std::string_view, factory_type>;

    static meta_factory_type meta_factory;

    static action_t * make_action(const std::string & action_type, const std::string & action_tag, const typename action_t::params_type & params) {
        try {
            return std::invoke(meta_factory.at(action_type), action_tag, params);
        }
        catch(const std::out_of_range & e) {
            spdlog::error("{} action is not in the meta factory", action_type);
            return std::invoke(meta_factory.at("End"), action_tag, params);
        }
    }
};

template<typename action_factory_t, typename context_t>
Stm<context_t, nlohmann::json>* make_stm_from_json(context_t & ctx, std::string filename, std::string dir, std::string meta_action_dir)
{
    using state_type = state<context_t, nlohmann::json>;

    std::string path = fmt::format("{}/{}", dir, filename);
    std::ifstream f(path);

    nlohmann::json json = nlohmann::json::parse(f);

    const auto make_action = [&](const nlohmann::json & j_action) -> std::pair<std::string, state_type *> {
        auto * new_state = [&]() -> state<context_t, nlohmann::json> * {
            if (j_action.contains("file"))
            {
                return make_stm_from_json<action_factory_t>(ctx, fmt::format("{}.json", j_action.at("file").get<std::string>()), meta_action_dir, meta_action_dir);
            }
            else
            {
                std::string action_type = j_action.at("action").get<std::string>();
                std::string action_tag = j_action.at("tag").get<std::string>();
                return action_factory_t::make_action(action_type, action_tag, j_action.value("parameters", nlohmann::json{}));
            }
        }();

        auto transitions = j_action.at("transitions")
                           | ranges::views::transform([&](const auto & j_transition) -> transition {
                                 return transition {
                                     j_transition.at("destination").template get<std::string>(),
                                     event{
                                         j_transition.at("type").template get<std::string>(),
                                         new_state->get_checksum()
                                     }
                                 };
                             })
                           | ranges::to<std::vector>;

        new_state->set_transitions(std::move(transitions));

        std::string tag = j_action.at("tag").get<std::string>();

        return std::pair{tag , new_state};
    };

    std::unordered_map<std::string, state_type*> states;
    state_type* entry_state;
    ranges::for_each(json.at("actions"), [&, first = true](const auto & j_action) mutable {
        auto action = make_action(j_action);
        if (first) {
            entry_state = action.second;
            first = false;
        }
        states.emplace(action);
    });

    return new Stm<context_t, nlohmann::json>(filename, ctx, entry_state, std::move(states));
}

}

#include <fmt/format.h>

template<>
struct fmt::formatter<nlohmann::json> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    static auto format(const nlohmann::json & val, format_context& ctx) {
        // Format val and write the output to ctx.out().
        return fmt::format_to(ctx.out(), "{} ", val.dump());
    }
};

