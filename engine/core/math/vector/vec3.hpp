#pragma once

#include <cmath>

#include "core/math/common/constants.hpp"
#include "core/math/common/math_utils.hpp"
#include "core/math/vector/vec2.hpp"

namespace spy::math {

struct Vec3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    constexpr Vec3() = default;
    constexpr Vec3(float xValue, float yValue, float zValue) : x(xValue), y(yValue), z(zValue) {}
    constexpr Vec3(const Vec2& xy, float zValue) : x(xy.x), y(xy.y), z(zValue) {}

    [[nodiscard]] static constexpr Vec3 zero()
    {
        return Vec3{0.0f, 0.0f, 0.0f};
    }

    [[nodiscard]] static constexpr Vec3 one()
    {
        return Vec3{1.0f, 1.0f, 1.0f};
    }

    [[nodiscard]] static constexpr Vec3 unitX()
    {
        return Vec3{1.0f, 0.0f, 0.0f};
    }

    [[nodiscard]] static constexpr Vec3 unitY()
    {
        return Vec3{0.0f, 1.0f, 0.0f};
    }

    [[nodiscard]] static constexpr Vec3 unitZ()
    {
        return Vec3{0.0f, 0.0f, 1.0f};
    }

    [[nodiscard]] constexpr Vec2 xy() const
    {
        return Vec2{x, y};
    }

    [[nodiscard]] constexpr Vec3 operator+(const Vec3& other) const
    {
        return Vec3{x + other.x, y + other.y, z + other.z};
    }

    [[nodiscard]] constexpr Vec3 operator-(const Vec3& other) const
    {
        return Vec3{x - other.x, y - other.y, z - other.z};
    }

    [[nodiscard]] constexpr Vec3 operator-() const
    {
        return Vec3{-x, -y, -z};
    }

    [[nodiscard]] constexpr Vec3 operator*(float scalar) const
    {
        return Vec3{x * scalar, y * scalar, z * scalar};
    }

    [[nodiscard]] constexpr Vec3 operator/(float scalar) const
    {
        return Vec3{x / scalar, y / scalar, z / scalar};
    }

    constexpr Vec3& operator+=(const Vec3& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    constexpr Vec3& operator-=(const Vec3& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    constexpr Vec3& operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    constexpr Vec3& operator/=(float scalar)
    {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    [[nodiscard]] constexpr bool operator==(const Vec3& other) const
    {
        return x == other.x && y == other.y && z == other.z;
    }

    [[nodiscard]] constexpr bool operator!=(const Vec3& other) const
    {
        return !(*this == other);
    }

    [[nodiscard]] constexpr float lengthSquared() const
    {
        return x * x + y * y + z * z;
    }

    [[nodiscard]] inline float length() const
    {
        return std::sqrt(lengthSquared());
    }

    [[nodiscard]] inline Vec3 normalized() const
    {
        const float squared = lengthSquared();

        if (squared <= constants::kEpsilon * constants::kEpsilon)
        {
            return zero();
        }

        const float inverseLength = 1.0f / std::sqrt(squared);
        return Vec3{x * inverseLength, y * inverseLength, z * inverseLength};
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
        return true;
    }

    [[nodiscard]] constexpr float dot(const Vec3& other) const
    {
        return x * other.x + y * other.y + z * other.z;
    }

    [[nodiscard]] constexpr Vec3 cross(const Vec3& other) const
    {
        return Vec3{y * other.z - z * other.y, z * other.x - x * other.z,
                     x * other.y - y * other.x};
    }

    [[nodiscard]] inline float distance(const Vec3& other) const
    {
        return (*this - other).length();
    }

    [[nodiscard]] static inline float distance(const Vec3& first, const Vec3& second)
    {
        return (first - second).length();
    }

    [[nodiscard]] constexpr float distanceSquared(const Vec3& other) const
    {
        return (*this - other).lengthSquared();
    }
};

[[nodiscard]] constexpr Vec3 operator*(float scalar, const Vec3& vector)
{
    return vector * scalar;
}

[[nodiscard]] inline bool nearlyEqual(const Vec3& first, const Vec3& second,
                                      float tolerance = constants::kEpsilon)
{
    return nearlyEqual(first.x, second.x, tolerance) && nearlyEqual(first.y, second.y, tolerance) &&
           nearlyEqual(first.z, second.z, tolerance);
}

[[nodiscard]] constexpr Vec3 project(const Vec3& vector, const Vec3& onto)
{
    const float lengthSquared = onto.lengthSquared();

    if (lengthSquared <= constants::kEpsilon * constants::kEpsilon)
    {
        return Vec3::zero();
    }

    return onto * (vector.dot(onto) / lengthSquared);
}

[[nodiscard]] constexpr Vec3 reflect(const Vec3& vector, const Vec3& normal)
{
    return vector - project(vector, normal) * 2.0f;
}

}
