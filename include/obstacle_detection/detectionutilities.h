#pragma once

#include <QPointF>
#include <QVector>
#include <cmath>

namespace DetectionUtilities
{
[[maybe_unused]] static QPointF calculatePosition(const QPointF &pos, int currentAngle, int dist, int angle)
{
    QPointF calculatedPos = {};
    double rad = (angle + currentAngle) * M_PI / 180.0;
    calculatedPos.setX(pos.x() + dist * std::cos(rad));
    calculatedPos.setY(pos.y() + dist * std::sin(rad));
    return calculatedPos;
}

[[maybe_unused]] static QPointF calculatePosition(const QPointF &pos, double currentRad, int dist, double rad)
{
    QPointF calculatedPos = {};
    rad += currentRad;
    calculatedPos.setX(pos.x() + dist * std::cos(rad));
    calculatedPos.setY(pos.y() + dist * std::sin(rad));
    return calculatedPos;
}
};
