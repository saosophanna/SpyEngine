#pragma once

#include "core/math/common/math_utils.hpp"
#include "core/math/vector/vec3.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::math {

struct Aabb
{
    Vec3 min{};
    Vec3 max{};

    [[nodiscard]] constexpr Vec3 center() const
    {
        return (min + max) * 0.5f;
    }

    [[nodiscard]] constexpr Vec3 extents() const
    {
        return (max - min) * 0.5f;
    }

    [[nodiscard]] constexpr Vec3 size() const
    {
        return max - min;
    }

    [[nodiscard]] constexpr float32 volume() const
    {
        const Vec3 dimensions = size();

        if (dimensions.x <= 0.0f || dimensions.y <= 0.0f || dimensions.z <= 0.0f)
        {
            return 0.0f;
        }

        return dimensions.x * dimensions.y * dimensions.z;
    }

    [[nodiscard]] constexpr bool contains(const Vec3& point) const
    {
        return point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }

    [[nodiscard]] constexpr bool contains(const Aabb& other) const
    {
        return other.min.x >= min.x && other.max.x <= max.x && other.min.y >= min.y &&
               other.max.y <= max.y && other.min.z >= min.z && other.max.z <= max.z;
    }

    [[nodiscard]] constexpr bool overlaps(const Aabb& other) const
    {
        return min.x <= other.max.x && max.x >= other.min.x && min.y <= other.max.y &&
               max.y >= other.min.y && min.z <= other.max.z && max.z >= other.min.z;
    }

    [[nodiscard]] constexpr Aabb merged(const Aabb& other) const
    {
        return Aabb{Vec3{minValue(min.x, other.min.x), minValue(min.y, other.min.y),
                         minValue(min.z, other.min.z)},
                    Vec3{maxValue(max.x, other.max.x), maxValue(max.y, other.max.y),
                         maxValue(max.z, other.max.z)}};
    }

    [[nodiscard]] constexpr Aabb expanded(float32 amount) const
    {
        return Aabb{Vec3{min.x - amount, min.y - amount, min.z - amount},
                    Vec3{max.x + amount, max.y + amount, max.z + amount}};
    }
};

}
