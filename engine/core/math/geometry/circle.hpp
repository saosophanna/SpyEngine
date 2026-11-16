#pragma once

#include "core/math/common/constants.hpp"
#include "core/math/vector/vec2.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::math {

struct Circle
{
    Vec2 center{};
    float32 radius = 0.0f;

    [[nodiscard]] constexpr bool contains(const Vec2& point) const
    {
        return point.distanceSquared(center) <= radius * radius;
    }

    [[nodiscard]] constexpr float32 area() const
    {
        return constants::kPi * radius * radius;
    }

    [[nodiscard]] constexpr float32 circumference() const
    {
        return constants::kTwoPi * radius;
    }
};

}
