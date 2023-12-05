#pragma once

#include <string>

struct Event
{
    std::string value;
    uint16_t checksum = 0xFFFF;
};

constexpr bool operator==(const Event &lhs, const Event &rhs) {
    return (lhs.value == rhs.value) && (rhs.checksum == 0xFFFF || lhs.checksum == rhs.checksum);
}
