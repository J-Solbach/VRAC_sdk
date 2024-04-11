#pragma once

#include <QPointF>
#include <QVector>
#include <cmath>

namespace DetectionUtilities
{

struct degrees {};
struct radians {};

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

}
