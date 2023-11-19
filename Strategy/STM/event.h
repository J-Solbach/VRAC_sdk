#ifndef EVENT_H
#define EVENT_H

#include <QString>
#include <QObject>

struct Event
{
    std::string value;
    uint16_t checksum = 0xFFFF;
};

constexpr bool operator==(const Event &lhs, const Event &rhs) {
    return (lhs.value == rhs.value) && (rhs.checksum == 0xFFFF || lhs.checksum == rhs.checksum);
}

#endif // EVENT_H
