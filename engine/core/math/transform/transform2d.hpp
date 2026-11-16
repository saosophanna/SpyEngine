#pragma once

#include <cmath>

#include "core/math/common/constants.hpp"
#include "core/math/common/math_utils.hpp"
#include "core/math/matrix/mat3.hpp"
#include "core/math/vector/vec2.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::math {

struct Transform2D
{
    Vec2 position{};
    float32 rotation = 0.0f;
    Vec2 scale{1.0f, 1.0f};

    [[nodiscard]] inline Mat3 matrix() const
    {
        return Mat3::translation(position) * Mat3::rotation(rotation) * Mat3::scaling(scale);
    }

    [[nodiscard]] inline Vec2 transformPoint(const Vec2& point) const
    {
        const Vec2 scaled{point.x * scale.x, point.y * scale.y};
        return position + scaled.rotated(rotation);
    }

    [[nodiscard]] inline Vec2 transformVector(const Vec2& vector) const
    {
        return Vec2{vector.x * scale.x, vector.y * scale.y}.rotated(rotation);
    }

    [[nodiscard]] inline bool inverseTransformPoint(const Vec2& point, Vec2& result) const
    {
        if (absValue(scale.x) <= constants::kEpsilon || absValue(scale.y) <= constants::kEpsilon)
        {
            return false;
        }

        const Vec2 local = (point - position).rotated(-rotation);
        result = Vec2{local.x / scale.x, local.y / scale.y};
        return true;
    }
};

}
