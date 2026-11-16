#pragma once

#include "core/math/common/constants.hpp"
#include "core/math/common/math_utils.hpp"
#include "core/math/vector/vec3.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::math {

struct Plane
{
    Vec3 normal{0.0f, 1.0f, 0.0f};
    float32 distance = 0.0f;

    [[nodiscard]] static Plane fromPointNormal(const Vec3& point, const Vec3& normal)
    {
        const Vec3 normalizedNormal = normal.normalized();

        if (normalizedNormal.lengthSquared() <= constants::kEpsilon * constants::kEpsilon)
        {
            return Plane{};
        }

        return Plane{normalizedNormal, -normalizedNormal.dot(point)};
    }

    [[nodiscard]] constexpr float32 signedDistance(const Vec3& point) const
    {
        return normal.dot(point) + distance;
    }

    [[nodiscard]] constexpr Vec3 projectPoint(const Vec3& point) const
    {
        const float32 lengthSquared = normal.lengthSquared();

        if (lengthSquared <= constants::kEpsilon * constants::kEpsilon)
        {
            return point;
        }

        return point - normal * (signedDistance(point) / lengthSquared);
    }

    [[nodiscard]] inline Plane normalized() const
    {
        const float32 length = normal.length();

        if (length <= constants::kEpsilon)
        {
            return Plane{};
        }

        return Plane{normal / length, distance / length};
    }
};

}
