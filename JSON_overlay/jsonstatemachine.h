#ifndef JSONSTATEMACHINE_H
#define JSONSTATEMACHINE_H

#include <fstream>
#include <nlohmann/json.hpp>
#include "Strategy/STM/stm.h"
#include "../Strategy/STM/transition.h"

template<typename action_factory_t, typename context_t>
Stm<State<context_t, nlohmann::json>>* make_stm_from_json(action_factory_t && action_factory, std::string filename, std::string dir)
{
    using state_type = State<context_t, nlohmann::json>;

    std::string path = dir + filename + ".json";
    std::ifstream f(path);
    nlohmann::json json = nlohmann::json::parse(f);

    const auto make_action = [&](const nlohmann::json & j_action) -> std::pair<std::string, state_type *> {
        auto * newState = [&]() -> State<context_t, nlohmann::json> {
            if (j_action.contains("file"))
            {
                return make_stm_from_json<context_t>(j_action.at("file").get<std::string>(), dir);
            }
            else
            {
                std::string actionTag = j_action.at("tag").get<std::string>();
                return action_factory->getAction(actionTag,  j_action);
            }
        }();

        auto transitions = json.at("transitions")
                           | ranges::views::transform([&](const auto & j_transition) -> transition {
                                 return transition {
                                     j_transition.at("Destination").template get<std::string>(),
                                     Event{
                                         j_transition.at("type").template get<std::string>(),
                                         newState->checkSum()
                                     }
                                 };
                             })
                           | ranges::to<std::vector>;

        newState->setTransitions(std::move(transitions));

        std::string tag = j_action.at("tag").get<std::string>();

        return std::pair{tag , newState};
    };

    auto * stm = new Stm<state_type>(filename);

    std::unordered_map<std::string, state_type*> states;

    ranges::for_each(json.at("actions"), [&](const auto & j_action){
        states.emplace(make_action(j_action));
    });

    stm->setStates(std::move(states));

    return stm;
}

#endif // JSONSTATEMACHINE_H
