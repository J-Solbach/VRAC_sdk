#pragma once

#include "qt_graphics_models/obstacle.h"

#include "path_finding/path_planner.h"
#include "path_finding/path_step.h"
#include <string>

namespace vrac::qt_graphics::models {

struct game_element
{
    std::string name;
    std::vector<QPointF> entry_points;
    vrac::qt_graphics::models::obstacle hitbox;
    std::vector<std::string> blocked_by;
    std::size_t collision_count = 0;
};

}

#include <nlohmann/json.hpp>

namespace nlohmann {

template<>
struct adl_serializer<vrac::qt_graphics::models::game_element> {
    static vrac::qt_graphics::models::game_element from_json(const json & j) {
        return {
            .name = j["name"],
            .entry_points = j["entry_points"],
            .hitbox = j,
            .blocked_by = j["blocked_by"]
        };
    }
};

}