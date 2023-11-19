#ifndef ACTIONFACTORY_H
#define ACTIONFACTORY_H

#include <unordered_map>
#include <string>

template<typename action_t>
struct ActionFactory {
    std::unordered_map<std::string, std::function<action_t *(std::string, typename action_t::param_t)>> map;
};

#endif // ACTIONFACTORY_H
