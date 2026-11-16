#pragma once

#include <limits>

#include "core/math/common/math_utils.hpp"
#include "core/math/geometry/aabb.hpp"
#include "core/math/vector/vec3.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::math {

struct Bounds
{
    Vec3 min{std::numeric_limits<float32>::max(), std::numeric_limits<float32>::max(),
             std::numeric_limits<float32>::max()};
    Vec3 max{std::numeric_limits<float32>::lowest(), std::numeric_limits<float32>::lowest(),
             std::numeric_limits<float32>::lowest()};

    [[nodiscard]] constexpr bool valid() const
    {
        return min.x <= max.x && min.y <= max.y && min.z <= max.z;
    }

    void reset()
    {
        min = Vec3{std::numeric_limits<float32>::max(), std::numeric_limits<float32>::max(),
                   std::numeric_limits<float32>::max()};
        max = Vec3{std::numeric_limits<float32>::lowest(), std::numeric_limits<float32>::lowest(),
                   std::numeric_limits<float32>::lowest()};
    }

    void expand(const Vec3& point)
    {
        min = Vec3{minValue(min.x, point.x), minValue(min.y, point.y), minValue(min.z, point.z)};
        max = Vec3{maxValue(max.x, point.x), maxValue(max.y, point.y), maxValue(max.z, point.z)};
    }

    void expand(const Aabb& box)
    {
        expand(box.min);
        expand(box.max);
    }

    [[nodiscard]] Vec3 center() const
    {
        return (min + max) * 0.5f;
    }

    [[nodiscard]] Vec3 size() const
    {
        return max - min;
    }

    [[nodiscard]] constexpr bool contains(const Vec3& point) const
    {
        return valid() && point.x >= min.x && point.x <= max.x && point.y >= min.y &&
               point.y <= max.y && point.z >= min.z && point.z <= max.z;
    }

    [[nodiscard]] constexpr Aabb toAabb() const
    {
        return Aabb{min, max};
    }
};

}
