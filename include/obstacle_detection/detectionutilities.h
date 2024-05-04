#pragma once

#include <QPointF>
#include <QVector>
#include <cmath>
#include <spdlog/spdlog.h>

namespace DetectionUtilities
{

struct degrees {};
struct radians {};
struct robotCoordinateSystem {};

template<typename theta_type>
[[maybe_unused]] QPointF calculatePosition(double dist, double angle)
{
    return QPointF();
}

template<>
[[maybe_unused]] inline QPointF calculatePosition<radians>(double dist, double rad)
{
    return QPointF(dist * std::cos(rad), dist * std::sin(rad));
}

template<>
[[maybe_unused]] inline QPointF calculatePosition<degrees>(double dist, double angle)
{
    double rad = (angle) * M_PI / 180.0;
    return calculatePosition<radians>(dist, rad);
}

template<>
[[maybe_unused]] inline QPointF calculatePosition<robotCoordinateSystem>(double dist, double angle)
{
    double rad = (-angle +  90) * M_PI / 180.0;
    return calculatePosition<radians>(dist, rad);
}

}
