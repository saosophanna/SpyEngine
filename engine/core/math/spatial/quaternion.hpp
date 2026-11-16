#pragma once

#include <cmath>
#include <cstddef>

#include "core/math/common/constants.hpp"
#include "core/math/common/math_utils.hpp"
#include "core/math/matrix/mat3.hpp"
#include "core/math/matrix/mat4.hpp"
#include "core/math/vector/vec3.hpp"

namespace spy::math {

struct Quaternion
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;

    constexpr Quaternion() = default;
    constexpr Quaternion(float xValue, float yValue, float zValue, float wValue)
        : x(xValue), y(yValue), z(zValue), w(wValue)
    {
    }

    [[nodiscard]] static constexpr Quaternion identity()
    {
        return Quaternion{};
    }

    [[nodiscard]] static inline Quaternion fromAxisAngle(const Vec3& axis, float radians)
    {
        const Vec3 normalizedAxis = axis.normalized();

        if (normalizedAxis.lengthSquared() <= constants::kEpsilon * constants::kEpsilon)
        {
            return identity();
        }

        const float halfAngle = radians * 0.5f;
        const float sine = std::sin(halfAngle);
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

    [[nodiscard]] constexpr Quaternion operator*(float scalar) const
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

    [[nodiscard]] constexpr float lengthSquared() const
    {
        return x * x + y * y + z * z + w * w;
    }

    [[nodiscard]] inline float length() const
    {
        return std::sqrt(lengthSquared());
    }

    [[nodiscard]] inline Quaternion normalized() const
    {
        const float squared = lengthSquared();

        if (squared <= constants::kEpsilon * constants::kEpsilon)
        {
            return identity();
        }

        const float inverseLength = 1.0f / std::sqrt(squared);
        return Quaternion{x * inverseLength, y * inverseLength, z * inverseLength,
                          w * inverseLength};
    }

    inline bool normalize()
    {
        const float squared = lengthSquared();

        if (squared <= constants::kEpsilon * constants::kEpsilon)
        {
            return false;
        }

        const float inverseLength = 1.0f / std::sqrt(squared);
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
        const float squared = lengthSquared();

        if (squared <= constants::kEpsilon * constants::kEpsilon)
        {
            return identity();
        }

        const float inverseLength = 1.0f / squared;
        return Quaternion{-x * inverseLength, -y * inverseLength, -z * inverseLength,
                          w * inverseLength};
    }

    [[nodiscard]] constexpr float dot(const Quaternion& other) const
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
        const float xx = x * x;
        const float yy = y * y;
        const float zz = z * z;
        const float xy = x * y;
        const float xz = x * z;
        const float yz = y * z;
        const float wx = w * x;
        const float wy = w * y;
        const float wz = w * z;

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

        for (std::size_t column = 0; column < 3; ++column)
        {
            for (std::size_t row = 0; row < 3; ++row)
            {
                result.element(column, row) = rotationMatrix.element(column, row);
            }
        }

        return result;
    }
};

[[nodiscard]] inline Quaternion slerp(const Quaternion& start, const Quaternion& end, float amount)
{
    float cosine = start.dot(end);
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

    const float angle = std::acos(clamp(cosine, -1.0f, 1.0f));
    const float sine = std::sin(angle);
    const float startScale = std::sin((1.0f - amount) * angle) / sine;
    const float endScale = std::sin(amount * angle) / sine;

    return Quaternion{start.x * startScale + adjustedEnd.x * endScale,
                      start.y * startScale + adjustedEnd.y * endScale,
                      start.z * startScale + adjustedEnd.z * endScale,
                      start.w * startScale + adjustedEnd.w * endScale};
}

[[nodiscard]] inline bool nearlyEqual(const Quaternion& first, const Quaternion& second,
                                      float tolerance = constants::kEpsilon)
{
    return nearlyEqual(first.x, second.x, tolerance) && nearlyEqual(first.y, second.y, tolerance) &&
           nearlyEqual(first.z, second.z, tolerance) && nearlyEqual(first.w, second.w, tolerance);
}

}
