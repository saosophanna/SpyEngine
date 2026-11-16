#pragma once

#include <cmath>

#include "core/math/common/constants.hpp"
#include "core/math/common/math_utils.hpp"
#include "core/math/vector/vec3.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::math {

struct Vec4
{
    float32 x = 0.0f;
    float32 y = 0.0f;
    float32 z = 0.0f;
    float32 w = 0.0f;

    constexpr Vec4() = default;
    constexpr Vec4(float32 xValue, float32 yValue, float32 zValue, float32 wValue)
        : x(xValue), y(yValue), z(zValue), w(wValue)
    {
    }

    constexpr Vec4(const Vec3& xyz, float32 wValue) : x(xyz.x), y(xyz.y), z(xyz.z), w(wValue) {}

    [[nodiscard]] static constexpr Vec4 zero()
    {
        return Vec4{0.0f, 0.0f, 0.0f, 0.0f};
    }

    [[nodiscard]] static constexpr Vec4 one()
    {
        return Vec4{1.0f, 1.0f, 1.0f, 1.0f};
    }

    [[nodiscard]] static constexpr Vec4 unitX()
    {
        return Vec4{1.0f, 0.0f, 0.0f, 0.0f};
    }

    [[nodiscard]] static constexpr Vec4 unitY()
    {
        return Vec4{0.0f, 1.0f, 0.0f, 0.0f};
    }

    [[nodiscard]] static constexpr Vec4 unitZ()
    {
        return Vec4{0.0f, 0.0f, 1.0f, 0.0f};
    }

    [[nodiscard]] static constexpr Vec4 unitW()
    {
        return Vec4{0.0f, 0.0f, 0.0f, 1.0f};
    }

    [[nodiscard]] constexpr Vec3 xyz() const
    {
        return Vec3{x, y, z};
    }

    [[nodiscard]] constexpr Vec4 operator+(const Vec4& other) const
    {
        return Vec4{x + other.x, y + other.y, z + other.z, w + other.w};
    }

    [[nodiscard]] constexpr Vec4 operator-(const Vec4& other) const
    {
        return Vec4{x - other.x, y - other.y, z - other.z, w - other.w};
    }

    [[nodiscard]] constexpr Vec4 operator-() const
    {
        return Vec4{-x, -y, -z, -w};
    }

    [[nodiscard]] constexpr Vec4 operator*(float32 scalar) const
    {
        return Vec4{x * scalar, y * scalar, z * scalar, w * scalar};
    }

    [[nodiscard]] constexpr Vec4 operator/(float32 scalar) const
    {
        return Vec4{x / scalar, y / scalar, z / scalar, w / scalar};
    }

    constexpr Vec4& operator+=(const Vec4& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
        return *this;
    }

    constexpr Vec4& operator-=(const Vec4& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
        return *this;
    }

    constexpr Vec4& operator*=(float32 scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
        return *this;
    }

    constexpr Vec4& operator/=(float32 scalar)
    {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        w /= scalar;
        return *this;
    }

    [[nodiscard]] constexpr bool operator==(const Vec4& other) const
    {
        return x == other.x && y == other.y && z == other.z && w == other.w;
    }

    [[nodiscard]] constexpr bool operator!=(const Vec4& other) const
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

    [[nodiscard]] inline Vec4 normalized() const
    {
        const float32 squared = lengthSquared();

        if (squared <= constants::kEpsilon * constants::kEpsilon)
        {
            return zero();
        }

        const float32 inverseLength = 1.0f / std::sqrt(squared);
        return Vec4{x * inverseLength, y * inverseLength, z * inverseLength, w * inverseLength};
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

    [[nodiscard]] constexpr float32 dot(const Vec4& other) const
    {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }

    [[nodiscard]] inline float32 distance(const Vec4& other) const
    {
        return (*this - other).length();
    }

    [[nodiscard]] static inline float32 distance(const Vec4& first, const Vec4& second)
    {
        return (first - second).length();
    }

    [[nodiscard]] constexpr float32 distanceSquared(const Vec4& other) const
    {
        return (*this - other).lengthSquared();
    }
};

[[nodiscard]] constexpr Vec4 operator*(float32 scalar, const Vec4& vector)
{
    return vector * scalar;
}

[[nodiscard]] inline bool nearlyEqual(const Vec4& first, const Vec4& second,
                                      float32 tolerance = constants::kEpsilon)
{
    return nearlyEqual(first.x, second.x, tolerance) && nearlyEqual(first.y, second.y, tolerance) &&
           nearlyEqual(first.z, second.z, tolerance) && nearlyEqual(first.w, second.w, tolerance);
}

}
