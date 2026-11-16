#pragma once

#include <array>
#include <cmath>

#include "core/math/common/constants.hpp"
#include "core/math/common/math_utils.hpp"
#include "core/math/geometry/aabb.hpp"
#include "core/math/matrix/mat4.hpp"
#include "core/math/spatial/plane.hpp"
#include "core/math/vector/vec3.hpp"
#include "core/math/vector/vec4.hpp"

namespace spy::math::detail {

[[nodiscard]] inline Plane planeFromCoefficients(float x, float y, float z, float constant)
{
    const float length = std::sqrt(x * x + y * y + z * z);

    if (length <= constants::kEpsilon)
    {
        return Plane{};
    }

    return Plane{Vec3{x / length, y / length, z / length}, constant / length};
}

}

namespace spy::math {

struct Frustum
{
    std::array<Plane, 6> planes{};

    [[nodiscard]] static Frustum fromViewProjection(const Mat4& matrix)
    {
        return fromViewProjectionDepthRange(matrix, true);
    }

    [[nodiscard]] static Frustum fromViewProjectionOpenGl(const Mat4& matrix)
    {
        return fromViewProjectionDepthRange(matrix, false);
    }

    [[nodiscard]] inline bool containsPoint(const Vec3& point) const
    {
        for (const Plane& plane : planes)
        {
            if (plane.signedDistance(point) < 0.0f)
            {
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] inline bool intersectsSphere(const Vec3& center, float radius) const
    {
        for (const Plane& plane : planes)
        {
            if (plane.signedDistance(center) < -radius)
            {
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] inline bool intersectsAabb(const Aabb& box) const
    {
        for (const Plane& plane : planes)
        {
            const Vec3 positive{plane.normal.x >= 0.0f ? box.max.x : box.min.x,
                                plane.normal.y >= 0.0f ? box.max.y : box.min.y,
                                plane.normal.z >= 0.0f ? box.max.z : box.min.z};

            if (plane.signedDistance(positive) < 0.0f)
            {
                return false;
            }
        }

        return true;
    }

private:
    [[nodiscard]] static Frustum fromViewProjectionDepthRange(const Mat4& matrix,
                                                               bool zeroToOneDepth)
    {
        const Vec4 row0{matrix.element(0, 0), matrix.element(1, 0), matrix.element(2, 0),
                        matrix.element(3, 0)};
        const Vec4 row1{matrix.element(0, 1), matrix.element(1, 1), matrix.element(2, 1),
                        matrix.element(3, 1)};
        const Vec4 row2{matrix.element(0, 2), matrix.element(1, 2), matrix.element(2, 2),
                        matrix.element(3, 2)};
        const Vec4 row3{matrix.element(0, 3), matrix.element(1, 3), matrix.element(2, 3),
                        matrix.element(3, 3)};

        const Vec4 nearRow = zeroToOneDepth ? row2 : row3 + row2;

        Frustum result;
        result.planes[0] = detail::planeFromCoefficients(row3.x + row0.x, row3.y + row0.y,
                                                         row3.z + row0.z, row3.w + row0.w);
        result.planes[1] = detail::planeFromCoefficients(row3.x - row0.x, row3.y - row0.y,
                                                         row3.z - row0.z, row3.w - row0.w);
        result.planes[2] = detail::planeFromCoefficients(row3.x + row1.x, row3.y + row1.y,
                                                         row3.z + row1.z, row3.w + row1.w);
        result.planes[3] = detail::planeFromCoefficients(row3.x - row1.x, row3.y - row1.y,
                                                         row3.z - row1.z, row3.w - row1.w);
        result.planes[4] = detail::planeFromCoefficients(nearRow.x, nearRow.y, nearRow.z,
                                                         nearRow.w);
        result.planes[5] = detail::planeFromCoefficients(row3.x - row2.x, row3.y - row2.y,
                                                         row3.z - row2.z, row3.w - row2.w);
        return result;
    }
};

}
