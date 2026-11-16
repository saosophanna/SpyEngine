#pragma once

#include <array>
#include <cmath>
#include <cstddef>

#include "core/math/common/constants.hpp"
#include "core/math/common/math_utils.hpp"
#include "core/math/vector/vec2.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::math {

struct Mat3
{
    std::array<float32, 9> elements{1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};

    [[nodiscard]] static constexpr Mat3 identity()
    {
        return Mat3{};
    }

    [[nodiscard]] static inline Mat3 translation(const Vec2& offset)
    {
        Mat3 result;
        result.element(2, 0) = offset.x;
        result.element(2, 1) = offset.y;
        return result;
    }

    [[nodiscard]] static inline Mat3 rotation(float32 radians)
    {
        const float32 cosine = std::cos(radians);
        const float32 sine = std::sin(radians);

        Mat3 result;
        result.element(0, 0) = cosine;
        result.element(0, 1) = sine;
        result.element(1, 0) = -sine;
        result.element(1, 1) = cosine;
        return result;
    }

    [[nodiscard]] static constexpr Mat3 scaling(const Vec2& factor)
    {
        Mat3 result;
        result.element(0, 0) = factor.x;
        result.element(1, 1) = factor.y;
        return result;
    }

    [[nodiscard]] constexpr float32 element(usize column, usize row) const
    {
        return elements[column * 3 + row];
    }

    constexpr float32& element(usize column, usize row)
    {
        return elements[column * 3 + row];
    }

    [[nodiscard]] constexpr Mat3 operator*(const Mat3& other) const
    {
        Mat3 result;

        for (usize column = 0; column < 3; ++column)
        {
            for (usize row = 0; row < 3; ++row)
            {
                float32 sum = 0.0f;

                for (usize index = 0; index < 3; ++index)
                {
                    sum += element(index, row) * other.element(column, index);
                }

                result.element(column, row) = sum;
            }
        }

        return result;
    }

    constexpr Mat3& operator*=(const Mat3& other)
    {
        *this = *this * other;
        return *this;
    }

    [[nodiscard]] constexpr Vec2 transformPoint(const Vec2& point) const
    {
        return Vec2{element(0, 0) * point.x + element(1, 0) * point.y + element(2, 0),
                    element(0, 1) * point.x + element(1, 1) * point.y + element(2, 1)};
    }

    [[nodiscard]] constexpr Vec2 transformVector(const Vec2& vector) const
    {
        return Vec2{element(0, 0) * vector.x + element(1, 0) * vector.y,
                    element(0, 1) * vector.x + element(1, 1) * vector.y};
    }

    [[nodiscard]] constexpr Mat3 transposed() const
    {
        Mat3 result;

        for (usize column = 0; column < 3; ++column)
        {
            for (usize row = 0; row < 3; ++row)
            {
                result.element(column, row) = element(row, column);
            }
        }

        return result;
    }

    [[nodiscard]] constexpr float32 determinant() const
    {
        return element(0, 0) * element(1, 1) - element(1, 0) * element(0, 1);
    }

    [[nodiscard]] inline bool inverse(Mat3& result) const
    {
        const float32 determinantValue = determinant();

        if (absValue(determinantValue) <= constants::kEpsilon)
        {
            return false;
        }

        const float32 inverseDeterminant = 1.0f / determinantValue;

        Mat3 inverseMatrix;
        inverseMatrix.element(0, 0) = element(1, 1) * inverseDeterminant;
        inverseMatrix.element(0, 1) = -element(0, 1) * inverseDeterminant;
        inverseMatrix.element(1, 0) = -element(1, 0) * inverseDeterminant;
        inverseMatrix.element(1, 1) = element(0, 0) * inverseDeterminant;

        const Vec2 translationValue{element(2, 0), element(2, 1)};
        const Vec2 inverseTranslation = inverseMatrix.transformVector(translationValue);
        inverseMatrix.element(2, 0) = -inverseTranslation.x;
        inverseMatrix.element(2, 1) = -inverseTranslation.y;

        result = inverseMatrix;
        return true;
    }
};

[[nodiscard]] inline bool nearlyEqual(const Mat3& first, const Mat3& second,
                                      float32 tolerance = constants::kEpsilon)
{
    for (usize index = 0; index < first.elements.size(); ++index)
    {
        if (!nearlyEqual(first.elements[index], second.elements[index], tolerance))
        {
            return false;
        }
    }

    return true;
}

}
