#pragma once

#include <array>
#include <cmath>
#include <cstddef>

#include "core/math/common/constants.hpp"
#include "core/math/common/math_utils.hpp"
#include "core/math/vector/vec3.hpp"
#include "core/math/vector/vec4.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::math {

struct Mat4
{
    std::array<float32, 16> elements{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                   0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

    [[nodiscard]] static constexpr Mat4 identity()
    {
        return Mat4{};
    }

    [[nodiscard]] static constexpr Mat4 translation(const Vec3& offset)
    {
        Mat4 result;
        result.element(3, 0) = offset.x;
        result.element(3, 1) = offset.y;
        result.element(3, 2) = offset.z;
        return result;
    }

    [[nodiscard]] static inline Mat4 rotation(float32 radians, const Vec3& axis)
    {
        const Vec3 normalizedAxis = axis.normalized();

        if (normalizedAxis.lengthSquared() <= constants::kEpsilon * constants::kEpsilon)
        {
            return identity();
        }

        const float32 cosine = std::cos(radians);
        const float32 sine = std::sin(radians);
        const float32 complement = 1.0f - cosine;

        const float32 xx = normalizedAxis.x * normalizedAxis.x;
        const float32 yy = normalizedAxis.y * normalizedAxis.y;
        const float32 zz = normalizedAxis.z * normalizedAxis.z;
        const float32 xy = normalizedAxis.x * normalizedAxis.y;
        const float32 xz = normalizedAxis.x * normalizedAxis.z;
        const float32 yz = normalizedAxis.y * normalizedAxis.z;

        Mat4 result;
        result.element(0, 0) = complement * xx + cosine;
        result.element(0, 1) = complement * xy + sine * normalizedAxis.z;
        result.element(0, 2) = complement * xz - sine * normalizedAxis.y;
        result.element(1, 0) = complement * xy - sine * normalizedAxis.z;
        result.element(1, 1) = complement * yy + cosine;
        result.element(1, 2) = complement * yz + sine * normalizedAxis.x;
        result.element(2, 0) = complement * xz + sine * normalizedAxis.y;
        result.element(2, 1) = complement * yz - sine * normalizedAxis.x;
        result.element(2, 2) = complement * zz + cosine;
        return result;
    }

    [[nodiscard]] static inline Mat4 rotationX(float32 radians)
    {
        const float32 cosine = std::cos(radians);
        const float32 sine = std::sin(radians);

        Mat4 result;
        result.element(1, 1) = cosine;
        result.element(1, 2) = sine;
        result.element(2, 1) = -sine;
        result.element(2, 2) = cosine;
        return result;
    }

    [[nodiscard]] static inline Mat4 rotationY(float32 radians)
    {
        const float32 cosine = std::cos(radians);
        const float32 sine = std::sin(radians);

        Mat4 result;
        result.element(0, 0) = cosine;
        result.element(0, 2) = -sine;
        result.element(2, 0) = sine;
        result.element(2, 2) = cosine;
        return result;
    }

    [[nodiscard]] static inline Mat4 rotationZ(float32 radians)
    {
        const float32 cosine = std::cos(radians);
        const float32 sine = std::sin(radians);

        Mat4 result;
        result.element(0, 0) = cosine;
        result.element(0, 1) = sine;
        result.element(1, 0) = -sine;
        result.element(1, 1) = cosine;
        return result;
    }

    [[nodiscard]] static constexpr Mat4 scaling(const Vec3& factor)
    {
        Mat4 result;
        result.element(0, 0) = factor.x;
        result.element(1, 1) = factor.y;
        result.element(2, 2) = factor.z;
        return result;
    }

    [[nodiscard]] static inline Mat4 perspective(float32 fieldOfViewRadians, float32 aspect,
                                                 float32 nearPlane, float32 farPlane)
    {
        if (fieldOfViewRadians <= 0.0f || aspect <= 0.0f || nearPlane <= 0.0f ||
            farPlane <= nearPlane)
        {
            return identity();
        }

        const float32 focal = 1.0f / std::tan(fieldOfViewRadians * 0.5f);
        const float32 depthScale = farPlane / (nearPlane - farPlane);
        const float32 depthShift = farPlane * nearPlane / (nearPlane - farPlane);

        Mat4 result;
        result.element(0, 0) = focal / aspect;
        result.element(1, 1) = focal;
        result.element(2, 2) = depthScale;
        result.element(2, 3) = -1.0f;
        result.element(3, 2) = depthShift;
        result.element(3, 3) = 0.0f;
        return result;
    }

    [[nodiscard]] static inline Mat4 perspectiveOpenGl(float32 fieldOfViewRadians, float32 aspect,
                                                       float32 nearPlane, float32 farPlane)
    {
        if (fieldOfViewRadians <= 0.0f || aspect <= 0.0f || nearPlane <= 0.0f ||
            farPlane <= nearPlane)
        {
            return identity();
        }

        const float32 focal = 1.0f / std::tan(fieldOfViewRadians * 0.5f);

        Mat4 result;
        result.element(0, 0) = focal / aspect;
        result.element(1, 1) = focal;
        result.element(2, 2) = (farPlane + nearPlane) / (nearPlane - farPlane);
        result.element(2, 3) = -1.0f;
        result.element(3, 2) = 2.0f * farPlane * nearPlane / (nearPlane - farPlane);
        result.element(3, 3) = 0.0f;
        return result;
    }

    [[nodiscard]] static inline Mat4 orthographic(float32 left, float32 right, float32 bottom,
                                                  float32 top, float32 nearPlane, float32 farPlane)
    {
        if (right <= left || top <= bottom || farPlane <= nearPlane)
        {
            return identity();
        }

        Mat4 result;
        result.element(0, 0) = 2.0f / (right - left);
        result.element(1, 1) = 2.0f / (top - bottom);
        result.element(2, 2) = 1.0f / (nearPlane - farPlane);
        result.element(3, 0) = -(right + left) / (right - left);
        result.element(3, 1) = -(top + bottom) / (top - bottom);
        result.element(3, 2) = nearPlane / (nearPlane - farPlane);
        return result;
    }

    [[nodiscard]] static inline Mat4 orthographicOpenGl(float32 left, float32 right,
                                                        float32 bottom, float32 top,
                                                        float32 nearPlane, float32 farPlane)
    {
        if (right <= left || top <= bottom || farPlane <= nearPlane)
        {
            return identity();
        }

        Mat4 result;
        result.element(0, 0) = 2.0f / (right - left);
        result.element(1, 1) = 2.0f / (top - bottom);
        result.element(2, 2) = -2.0f / (farPlane - nearPlane);
        result.element(3, 0) = -(right + left) / (right - left);
        result.element(3, 1) = -(top + bottom) / (top - bottom);
        result.element(3, 2) = -(farPlane + nearPlane) / (farPlane - nearPlane);
        return result;
    }

    [[nodiscard]] static inline Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up)
    {
        const Vec3 forward = (target - eye).normalized();

        if (forward.lengthSquared() <= constants::kEpsilon * constants::kEpsilon)
        {
            return identity();
        }

        const Vec3 right = forward.cross(up).normalized();

        if (right.lengthSquared() <= constants::kEpsilon * constants::kEpsilon)
        {
            return identity();
        }

        const Vec3 correctedUp = right.cross(forward);

        Mat4 result;
        result.element(0, 0) = right.x;
        result.element(1, 0) = right.y;
        result.element(2, 0) = right.z;
        result.element(3, 0) = -right.dot(eye);

        result.element(0, 1) = correctedUp.x;
        result.element(1, 1) = correctedUp.y;
        result.element(2, 1) = correctedUp.z;
        result.element(3, 1) = -correctedUp.dot(eye);

        result.element(0, 2) = -forward.x;
        result.element(1, 2) = -forward.y;
        result.element(2, 2) = -forward.z;
        result.element(3, 2) = forward.dot(eye);
        return result;
    }

    [[nodiscard]] constexpr float32 element(usize column, usize row) const
    {
        return elements[column * 4 + row];
    }

    constexpr float32& element(usize column, usize row)
    {
        return elements[column * 4 + row];
    }

    [[nodiscard]] constexpr Mat4 operator*(const Mat4& other) const
    {
        Mat4 result;

        for (usize column = 0; column < 4; ++column)
        {
            for (usize row = 0; row < 4; ++row)
            {
                float32 sum = 0.0f;

                for (usize index = 0; index < 4; ++index)
                {
                    sum += element(index, row) * other.element(column, index);
                }

                result.element(column, row) = sum;
            }
        }

        return result;
    }

    constexpr Mat4& operator*=(const Mat4& other)
    {
        *this = *this * other;
        return *this;
    }

    [[nodiscard]] constexpr Vec4 transform(const Vec4& vector) const
    {
        return Vec4{element(0, 0) * vector.x + element(1, 0) * vector.y +
                        element(2, 0) * vector.z + element(3, 0) * vector.w,
                    element(0, 1) * vector.x + element(1, 1) * vector.y +
                        element(2, 1) * vector.z + element(3, 1) * vector.w,
                    element(0, 2) * vector.x + element(1, 2) * vector.y +
                        element(2, 2) * vector.z + element(3, 2) * vector.w,
                    element(0, 3) * vector.x + element(1, 3) * vector.y +
                        element(2, 3) * vector.z + element(3, 3) * vector.w};
    }

    [[nodiscard]] inline Vec3 transformPoint(const Vec3& point) const
    {
        const Vec4 transformed = transform(Vec4{point, 1.0f});

        if (absValue(transformed.w) > constants::kEpsilon)
        {
            const float32 inverseW = 1.0f / transformed.w;
            return Vec3{transformed.x * inverseW, transformed.y * inverseW,
                        transformed.z * inverseW};
        }

        return transformed.xyz();
    }

    [[nodiscard]] constexpr Vec3 transformVector(const Vec3& vector) const
    {
        return Vec3{element(0, 0) * vector.x + element(1, 0) * vector.y + element(2, 0) * vector.z,
                    element(0, 1) * vector.x + element(1, 1) * vector.y + element(2, 1) * vector.z,
                    element(0, 2) * vector.x + element(1, 2) * vector.y + element(2, 2) * vector.z};
    }

    [[nodiscard]] constexpr Mat4 transposed() const
    {
        Mat4 result;

        for (usize column = 0; column < 4; ++column)
        {
            for (usize row = 0; row < 4; ++row)
            {
                result.element(column, row) = element(row, column);
            }
        }

        return result;
    }

    [[nodiscard]] inline bool inverse(Mat4& result) const
    {
        float64 augmented[4][8] = {};

        for (usize row = 0; row < 4; ++row)
        {
            for (usize column = 0; column < 4; ++column)
            {
                augmented[row][column] = static_cast<float64>(element(column, row));
            }

            augmented[row][row + 4] = 1.0;
        }

        for (usize pivotIndex = 0; pivotIndex < 4; ++pivotIndex)
        {
            usize pivotRow = pivotIndex;
            float64 pivotMagnitude = std::fabs(augmented[pivotIndex][pivotIndex]);

            for (usize row = pivotIndex + 1; row < 4; ++row)
            {
                const float64 magnitude = std::fabs(augmented[row][pivotIndex]);

                if (magnitude > pivotMagnitude)
                {
                    pivotMagnitude = magnitude;
                    pivotRow = row;
                }
            }

            if (pivotMagnitude <= 1.0e-12)
            {
                return false;
            }

            if (pivotRow != pivotIndex)
            {
                for (usize column = 0; column < 8; ++column)
                {
                    const float64 temporary = augmented[pivotIndex][column];
                    augmented[pivotIndex][column] = augmented[pivotRow][column];
                    augmented[pivotRow][column] = temporary;
                }
            }

            const float64 pivot = augmented[pivotIndex][pivotIndex];

            for (usize column = 0; column < 8; ++column)
            {
                augmented[pivotIndex][column] /= pivot;
            }

            for (usize row = 0; row < 4; ++row)
            {
                if (row == pivotIndex)
                {
                    continue;
                }

                const float64 factor = augmented[row][pivotIndex];

                if (factor == 0.0)
                {
                    continue;
                }

                for (usize column = 0; column < 8; ++column)
                {
                    augmented[row][column] -= factor * augmented[pivotIndex][column];
                }
            }
        }

        for (usize row = 0; row < 4; ++row)
        {
            for (usize column = 0; column < 4; ++column)
            {
                result.element(column, row) = static_cast<float32>(augmented[row][column + 4]);
            }
        }

        return true;
    }
};

[[nodiscard]] inline bool nearlyEqual(const Mat4& first, const Mat4& second,
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
