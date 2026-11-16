#pragma once

#include <cmath>
#include <cstddef>

#include "core/math/common/constants.hpp"
#include "core/math/common/math_utils.hpp"
#include "core/math/matrix/mat3.hpp"
#include "core/math/matrix/mat4.hpp"
#include "core/math/vector/vec3.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::math {

struct Quaternion
{
    float32 x = 0.0f;
    float32 y = 0.0f;
    float32 z = 0.0f;
    float32 w = 1.0f;

    constexpr Quaternion() = default;
    constexpr Quaternion(float32 xValue, float32 yValue, float32 zValue, float32 wValue)
        : x(xValue), y(yValue), z(zValue), w(wValue)
    {
    }

    [[nodiscard]] static constexpr Quaternion identity()
    {
        return Quaternion{};
    }

    [[nodiscard]] static inline Quaternion fromAxisAngle(const Vec3& axis, float32 radians)
    {
        const Vec3 normalizedAxis = axis.normalized();

        if (normalizedAxis.lengthSquared() <= constants::kEpsilon * constants::kEpsilon)
        {
            return identity();
        }

        const float32 halfAngle = radians * 0.5f;
        const float32 sine = std::sin(halfAngle);
        return Quaternion{normalizedAxis.x * sine, normalizedAxis.y * sine,
                          normalizedAxis.z * sine, std::cos(halfAngle)};
    }

    [[nodiscard]] constexpr Vec3 vector() const
    {
        return Vec3{x, y, z};
    }

    [[nodiscard]] constexpr Quaternion operator-() const
    {
        return Quaternion{-x, -y, -z, -w};
    }

    [[nodiscard]] constexpr Quaternion operator+(const Quaternion& other) const
    {
        return Quaternion{x + other.x, y + other.y, z + other.z, w + other.w};
    }

    [[nodiscard]] constexpr Quaternion operator-(const Quaternion& other) const
    {
        return Quaternion{x - other.x, y - other.y, z - other.z, w - other.w};
    }

    [[nodiscard]] constexpr Quaternion operator*(float32 scalar) const
    {
        return Quaternion{x * scalar, y * scalar, z * scalar, w * scalar};
    }

    [[nodiscard]] constexpr Quaternion operator*(const Quaternion& other) const
    {
        return Quaternion{w * other.x + x * other.w + y * other.z - z * other.y,
                          w * other.y - x * other.z + y * other.w + z * other.x,
                          w * other.z + x * other.y - y * other.x + z * other.w,
                          w * other.w - x * other.x - y * other.y - z * other.z};
    }

    [[nodiscard]] constexpr bool operator==(const Quaternion& other) const
    {
        return x == other.x && y == other.y && z == other.z && w == other.w;
    }

    [[nodiscard]] constexpr bool operator!=(const Quaternion& other) const
    {
        return !(*this == other);
    }

    [[nodiscard]] constexpr float32 lengthSquared() const
    {
        return x * x + y * y + z * z + w * w;
    }

    [[nodiscard]] inline float32 length() const
    {
        return std::sqrt(lengthSquared());
    }

    [[nodiscard]] inline Quaternion normalized() const
    {
        const float32 squared = lengthSquared();

        if (squared <= constants::kEpsilon * constants::kEpsilon)
        {
            return identity();
        }

        const float32 inverseLength = 1.0f / std::sqrt(squared);
        return Quaternion{x * inverseLength, y * inverseLength, z * inverseLength,
                          w * inverseLength};
    }

    inline bool normalize()
    {
        const float32 squared = lengthSquared();

        if (squared <= constants::kEpsilon * constants::kEpsilon)
        {
            return false;
        }

        const float32 inverseLength = 1.0f / std::sqrt(squared);
        x *= inverseLength;
        y *= inverseLength;
        z *= inverseLength;
        w *= inverseLength;
        return true;
    }

    [[nodiscard]] constexpr Quaternion conjugate() const
    {
        return Quaternion{-x, -y, -z, w};
    }

    [[nodiscard]] inline Quaternion inverse() const
    {
        const float32 squared = lengthSquared();

        if (squared <= constants::kEpsilon * constants::kEpsilon)
        {
            return identity();
        }

        const float32 inverseLength = 1.0f / squared;
        return Quaternion{-x * inverseLength, -y * inverseLength, -z * inverseLength,
                          w * inverseLength};
    }

    [[nodiscard]] constexpr float32 dot(const Quaternion& other) const
    {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }

    [[nodiscard]] inline Vec3 rotate(const Vec3& vector) const
    {
        const Vec3 vectorPart{x, y, z};
        const Vec3 doubledCross = vectorPart.cross(vector) * 2.0f;
        return vector + doubledCross * w + vectorPart.cross(doubledCross);
    }

    [[nodiscard]] inline Mat3 toMat3() const
    {
        const float32 xx = x * x;
        const float32 yy = y * y;
        const float32 zz = z * z;
        const float32 xy = x * y;
        const float32 xz = x * z;
        const float32 yz = y * z;
        const float32 wx = w * x;
        const float32 wy = w * y;
        const float32 wz = w * z;

        Mat3 result;
        result.element(0, 0) = 1.0f - 2.0f * (yy + zz);
        result.element(0, 1) = 2.0f * (xy + wz);
        result.element(0, 2) = 2.0f * (xz - wy);
        result.element(1, 0) = 2.0f * (xy - wz);
        result.element(1, 1) = 1.0f - 2.0f * (xx + zz);
        result.element(1, 2) = 2.0f * (yz + wx);
        result.element(2, 0) = 2.0f * (xz + wy);
        result.element(2, 1) = 2.0f * (yz - wx);
        result.element(2, 2) = 1.0f - 2.0f * (xx + yy);
        return result;
    }

    [[nodiscard]] inline Mat4 toMat4() const
    {
        const Mat3 rotationMatrix = toMat3();

        Mat4 result;

        for (usize column = 0; column < 3; ++column)
        {
            for (usize row = 0; row < 3; ++row)
            {
                result.element(column, row) = rotationMatrix.element(column, row);
            }
        }

        return result;
    }
};

[[nodiscard]] inline Quaternion slerp(const Quaternion& start, const Quaternion& end,
                                      float32 amount)
{
    float32 cosine = start.dot(end);
    Quaternion adjustedEnd = end;

    if (cosine < 0.0f)
    {
        cosine = -cosine;
        adjustedEnd = -end;
    }

    if (cosine > 0.9995f)
    {
        return Quaternion{start.x + (adjustedEnd.x - start.x) * amount,
                          start.y + (adjustedEnd.y - start.y) * amount,
                          start.z + (adjustedEnd.z - start.z) * amount,
                          start.w + (adjustedEnd.w - start.w) * amount}
            .normalized();
    }

    const float32 angle = std::acos(clamp(cosine, -1.0f, 1.0f));
    const float32 sine = std::sin(angle);
    const float32 startScale = std::sin((1.0f - amount) * angle) / sine;
    const float32 endScale = std::sin(amount * angle) / sine;

    return Quaternion{start.x * startScale + adjustedEnd.x * endScale,
                      start.y * startScale + adjustedEnd.y * endScale,
                      start.z * startScale + adjustedEnd.z * endScale,
                      start.w * startScale + adjustedEnd.w * endScale};
}

[[nodiscard]] inline bool nearlyEqual(const Quaternion& first, const Quaternion& second,
                                      float32 tolerance = constants::kEpsilon)
{
    return nearlyEqual(first.x, second.x, tolerance) && nearlyEqual(first.y, second.y, tolerance) &&
           nearlyEqual(first.z, second.z, tolerance) && nearlyEqual(first.w, second.w, tolerance);
}

}
