#pragma once

#include "core/math/common/constants.hpp"
#include "core/math/vector/vec2.hpp"

namespace spy::math {

struct Circle
{
    Vec2 center{};
    float radius = 0.0f;

    [[nodiscard]] constexpr bool contains(const Vec2& point) const
    {
        return point.distanceSquared(center) <= radius * radius;
    }

    [[nodiscard]] constexpr float area() const
    {
        return constants::kPi * radius * radius;
    }

    [[nodiscard]] constexpr float circumference() const
    {
        return constants::kTwoPi * radius;
    }
};

}
