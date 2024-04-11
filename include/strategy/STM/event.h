#pragma once

#include <string>

namespace vrac::strategy::state_machines {

struct event
{
    std::string value;
    uint16_t checksum = 0xFFFF;
};

constexpr bool operator==(const event &lhs, const event &rhs) {
    return (lhs.value == rhs.value) && (rhs.checksum == 0xFFFF || lhs.checksum == rhs.checksum);
}
}
