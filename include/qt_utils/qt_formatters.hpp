#pragma once

#include <fmt/format.h>

template<>
struct fmt::formatter<QPointF> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    static auto format(const QPointF & val, format_context& ctx) {
        return fmt::format_to(ctx.out(), "(x:{}, y:{})", val.x(), val.y());
    }
};

template<>
struct fmt::formatter<QPolygonF> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    static auto format(const QPolygonF & val, format_context& ctx) {
        return fmt::format_to(ctx.out(), "({})", fmt::join(val.toVector(), ","));
    }
};

static inline void from_json(const nlohmann::json & j, QPointF & point) {
    point = QPointF(j["x"].get<double>(), j["y"].get<double>());
}