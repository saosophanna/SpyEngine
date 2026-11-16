#pragma once

#include <cmath>

#include "core/math/common/constants.hpp"
#include "core/math/common/math_utils.hpp"

namespace spy::math {

struct Vec2
{
    float x = 0.0f;
    float y = 0.0f;

    constexpr Vec2() = default;
    constexpr Vec2(float xValue, float yValue) : x(xValue), y(yValue) {}

    [[nodiscard]] static constexpr Vec2 zero()
    {
        return Vec2{0.0f, 0.0f};
    }

    [[nodiscard]] static constexpr Vec2 one()
    {
        return Vec2{1.0f, 1.0f};
    }

    [[nodiscard]] static constexpr Vec2 unitX()
    {
        return Vec2{1.0f, 0.0f};
    }

    [[nodiscard]] static constexpr Vec2 unitY()
    {
        return Vec2{0.0f, 1.0f};
    }

    [[nodiscard]] constexpr Vec2 operator+(const Vec2& other) const
    {
        return Vec2{x + other.x, y + other.y};
    }

    [[nodiscard]] constexpr Vec2 operator-(const Vec2& other) const
    {
        return Vec2{x - other.x, y - other.y};
    }

    [[nodiscard]] constexpr Vec2 operator-() const
    {
        return Vec2{-x, -y};
    }

    [[nodiscard]] constexpr Vec2 operator*(float scalar) const
    {
        return Vec2{x * scalar, y * scalar};
    }

    [[nodiscard]] constexpr Vec2 operator/(float scalar) const
    {
        return Vec2{x / scalar, y / scalar};
    }

    constexpr Vec2& operator+=(const Vec2& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr Vec2& operator-=(const Vec2& other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    constexpr Vec2& operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    constexpr Vec2& operator/=(float scalar)
    {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    [[nodiscard]] constexpr bool operator==(const Vec2& other) const
    {
        return x == other.x && y == other.y;
    }

    [[nodiscard]] constexpr bool operator!=(const Vec2& other) const
    {
        return !(*this == other);
    }

    [[nodiscard]] constexpr float lengthSquared() const
    {
        return x * x + y * y;
    }

    [[nodiscard]] inline float length() const
    {
        return std::sqrt(lengthSquared());
    }

    [[nodiscard]] inline Vec2 normalized() const
    {
        const float squared = lengthSquared();

        if (squared <= constants::kEpsilon * constants::kEpsilon)
        {
            return zero();
        }

        const float inverseLength = 1.0f / std::sqrt(squared);
        return Vec2{x * inverseLength, y * inverseLength};
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
        return true;
    }

    [[nodiscard]] constexpr float dot(const Vec2& other) const
    {
        return x * other.x + y * other.y;
    }

    [[nodiscard]] constexpr float cross(const Vec2& other) const
    {
        return x * other.y - y * other.x;
    }

    [[nodiscard]] inline float distance(const Vec2& other) const
    {
        return (*this - other).length();
    }

    [[nodiscard]] static inline float distance(const Vec2& first, const Vec2& second)
    {
        return (first - second).length();
    }

    [[nodiscard]] constexpr float distanceSquared(const Vec2& other) const
    {
        return (*this - other).lengthSquared();
    }

    [[nodiscard]] constexpr Vec2 perpendicular() const
    {
        return Vec2{-y, x};
    }

    [[nodiscard]] inline float angle() const
    {
        return std::atan2(y, x);
    }

    [[nodiscard]] inline Vec2 rotated(float radians) const
    {
        const float cosine = std::cos(radians);
        const float sine = std::sin(radians);
        return Vec2{x * cosine - y * sine, x * sine + y * cosine};
    }
};

[[nodiscard]] constexpr Vec2 operator*(float scalar, const Vec2& vector)
{
    return vector * scalar;
}

[[nodiscard]] inline bool nearlyEqual(const Vec2& first, const Vec2& second,
                                      float tolerance = constants::kEpsilon)
{
    return nearlyEqual(first.x, second.x, tolerance) && nearlyEqual(first.y, second.y, tolerance);
}

[[nodiscard]] constexpr Vec2 project(const Vec2& vector, const Vec2& onto)
{
    const float lengthSquared = onto.lengthSquared();

    if (lengthSquared <= constants::kEpsilon * constants::kEpsilon)
    {
        return Vec2::zero();
    }

    return onto * (vector.dot(onto) / lengthSquared);
}

[[nodiscard]] constexpr Vec2 reflect(const Vec2& vector, const Vec2& normal)
{
    return vector - project(vector, normal) * 2.0f;
}

[[nodiscard]] inline Vec2 rotate(const Vec2& vector, float radians)
{
    return vector.rotated(radians);
}

}
