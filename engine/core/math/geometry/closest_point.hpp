#pragma once

#include "core/math/common/constants.hpp"
#include "core/math/common/math_utils.hpp"
#include "core/math/geometry/aabb.hpp"
#include "core/math/geometry/rect.hpp"
#include "core/math/vector/vec2.hpp"
#include "core/math/vector/vec3.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::math {

[[nodiscard]] constexpr Vec2 closestPointOnRect(const Vec2& point, const Rect& rect)
{
    const Vec2 low = rect.min();
    const Vec2 high = rect.max();
    return Vec2{clamp(point.x, low.x, high.x), clamp(point.y, low.y, high.y)};
}

[[nodiscard]] constexpr Vec3 closestPointOnAabb(const Vec3& point, const Aabb& box)
{
    return Vec3{clamp(point.x, box.min.x, box.max.x), clamp(point.y, box.min.y, box.max.y),
                clamp(point.z, box.min.z, box.max.z)};
}

[[nodiscard]] constexpr Vec2 closestPointOnSegment(const Vec2& point, const Vec2& start,
                                                    const Vec2& end)
{
    const Vec2 edge = end - start;
    const float32 lengthSquared = edge.lengthSquared();

    if (lengthSquared <= constants::kEpsilon * constants::kEpsilon)
    {
        return start;
    }

    const float32 amount = clamp((point - start).dot(edge) / lengthSquared, 0.0f, 1.0f);
    return start + edge * amount;
}

}
