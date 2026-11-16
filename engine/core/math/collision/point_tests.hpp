#pragma once

#include "core/math/geometry/aabb.hpp"
#include "core/math/geometry/circle.hpp"
#include "core/math/geometry/rect.hpp"

namespace spy::math {

[[nodiscard]] inline bool pointInRect(const Vec2& point, const Rect& rect)
{
    return rect.contains(point);
}

[[nodiscard]] inline bool pointInCircle(const Vec2& point, const Circle& circle)
{
    return circle.contains(point);
}

[[nodiscard]] inline bool pointInAabb(const Vec3& point, const Aabb& box)
{
    return box.contains(point);
}

}
