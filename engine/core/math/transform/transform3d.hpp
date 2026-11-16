#pragma once

#include "core/math/common/constants.hpp"
#include "core/math/common/math_utils.hpp"
#include "core/math/matrix/mat4.hpp"
#include "core/math/spatial/quaternion.hpp"
#include "core/math/vector/vec3.hpp"

namespace spy::math {

struct Transform3D
{
    Vec3 position{};
    Quaternion rotation{};
    Vec3 scale{1.0f, 1.0f, 1.0f};

    [[nodiscard]] Mat4 matrix() const
    {
        return Mat4::translation(position) * rotation.toMat4() * Mat4::scaling(scale);
    }

    [[nodiscard]] Vec3 transformPoint(const Vec3& point) const
    {
        const Vec3 scaled{point.x * scale.x, point.y * scale.y, point.z * scale.z};
        return position + rotation.rotate(scaled);
    }

    [[nodiscard]] Vec3 transformVector(const Vec3& vector) const
    {
        return rotation.rotate(Vec3{vector.x * scale.x, vector.y * scale.y, vector.z * scale.z});
    }

    [[nodiscard]] bool inverseTransformPoint(const Vec3& point, Vec3& result) const
    {
        if (absValue(scale.x) <= constants::kEpsilon || absValue(scale.y) <= constants::kEpsilon ||
            absValue(scale.z) <= constants::kEpsilon)
        {
            return false;
        }

        const Vec3 local = rotation.inverse().rotate(point - position);
        result = Vec3{local.x / scale.x, local.y / scale.y, local.z / scale.z};
        return true;
    }
};

}
