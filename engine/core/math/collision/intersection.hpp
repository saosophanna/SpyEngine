#pragma once

#include <cmath>
#include <cstddef>
#include <limits>

#include "core/math/common/constants.hpp"
#include "core/math/common/math_utils.hpp"
#include "core/math/geometry/aabb.hpp"
#include "core/math/geometry/circle.hpp"
#include "core/math/geometry/closest_point.hpp"
#include "core/math/geometry/ray.hpp"
#include "core/math/geometry/rect.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::math {

[[nodiscard]] inline bool rectIntersectsRect(const Rect& first, const Rect& second)
{
    return first.overlaps(second);
}

[[nodiscard]] inline bool circleIntersectsCircle(const Circle& first, const Circle& second)
{
    const float32 radiusSum = first.radius + second.radius;
    return first.center.distanceSquared(second.center) <= radiusSum * radiusSum;
}

[[nodiscard]] inline bool circleIntersectsRect(const Circle& circle, const Rect& rect)
{
    const Vec2 closest = closestPointOnRect(circle.center, rect);
    return circle.center.distanceSquared(closest) <= circle.radius * circle.radius;
}

[[nodiscard]] inline bool aabbIntersectsAabb(const Aabb& first, const Aabb& second)
{
    return first.overlaps(second);
}

[[nodiscard]] inline bool rayIntersectsAabb(const Ray& ray, const Aabb& box, float32& distance)
{
    const float32 origins[3] = {ray.origin.x, ray.origin.y, ray.origin.z};
    const float32 directions[3] = {ray.direction.x, ray.direction.y, ray.direction.z};
    const float32 lows[3] = {box.min.x, box.min.y, box.min.z};
    const float32 highs[3] = {box.max.x, box.max.y, box.max.z};

    float32 entry = 0.0f;
    float32 exit = std::numeric_limits<float32>::max();

    for (usize axis = 0; axis < 3; ++axis)
    {
        if (absValue(directions[axis]) <= constants::kEpsilon)
        {
            if (origins[axis] < lows[axis] || origins[axis] > highs[axis])
            {
                return false;
            }

            continue;
        }

        float32 nearDistance = (lows[axis] - origins[axis]) / directions[axis];
        float32 farDistance = (highs[axis] - origins[axis]) / directions[axis];

        if (nearDistance > farDistance)
        {
            const float32 temporary = nearDistance;
            nearDistance = farDistance;
            farDistance = temporary;
        }

        entry = maxValue(entry, nearDistance);
        exit = minValue(exit, farDistance);

        if (entry > exit)
        {
            return false;
        }
    }

    distance = entry;
    return true;
}

[[nodiscard]] inline bool rayIntersectsCircle(const Ray2D& ray, const Circle& circle,
                                              float32& distance)
{
    const Vec2 offset = ray.origin - circle.center;
    const float32 quadratic = ray.direction.lengthSquared();

    if (quadratic <= constants::kEpsilon * constants::kEpsilon)
    {
        if (circle.contains(ray.origin))
        {
            distance = 0.0f;
            return true;
        }

        return false;
    }

    const float32 linear = 2.0f * offset.dot(ray.direction);
    const float32 constant = offset.lengthSquared() - circle.radius * circle.radius;
    const float32 discriminant = linear * linear - 4.0f * quadratic * constant;

    if (discriminant < 0.0f)
    {
        return false;
    }

    const float32 root = std::sqrt(discriminant);
    const float32 nearest = (-linear - root) / (2.0f * quadratic);

    if (nearest >= 0.0f)
    {
        distance = nearest;
        return true;
    }

    const float32 furthest = (-linear + root) / (2.0f * quadratic);

    if (furthest >= 0.0f)
    {
        distance = furthest;
        return true;
    }

    return false;
}

[[nodiscard]] inline bool segmentIntersectsSegment(const Vec2& firstStart, const Vec2& firstEnd,
                                                   const Vec2& secondStart, const Vec2& secondEnd,
                                                   float32& parameter)
{
    const Vec2 firstDirection = firstEnd - firstStart;
    const Vec2 secondDirection = secondEnd - secondStart;
    const float32 denominator = firstDirection.cross(secondDirection);

    if (absValue(denominator) <= constants::kEpsilon)
    {
        return false;
    }

    const Vec2 difference = secondStart - firstStart;
    const float32 firstParameter = difference.cross(secondDirection) / denominator;
    const float32 secondParameter = difference.cross(firstDirection) / denominator;

    if (firstParameter < 0.0f || firstParameter > 1.0f || secondParameter < 0.0f ||
        secondParameter > 1.0f)
    {
        return false;
    }

    parameter = firstParameter;
    return true;
}

}
