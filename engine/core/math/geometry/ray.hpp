#pragma once

#include "core/math/vector/vec2.hpp"
#include "core/math/vector/vec3.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::math {

struct Ray
{
    Vec3 origin{};
    Vec3 direction{0.0f, 0.0f, -1.0f};

    [[nodiscard]] constexpr Vec3 pointAt(float32 distance) const
    {
        return origin + direction * distance;
    }
};

struct Ray2D
{
    Vec2 origin{};
    Vec2 direction{1.0f, 0.0f};

    [[nodiscard]] constexpr Vec2 pointAt(float32 distance) const
    {
        return origin + direction * distance;
    }
};

}
