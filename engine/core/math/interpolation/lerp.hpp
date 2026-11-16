#pragma once

#include "core/math/common/constants.hpp"
#include "core/math/common/math_utils.hpp"
#include "core/math/vector/vec2.hpp"
#include "core/math/vector/vec3.hpp"
#include "core/math/vector/vec4.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::math {

[[nodiscard]] constexpr float32 lerp(float32 start, float32 end, float32 amount)
{
    return start + (end - start) * amount;
}

[[nodiscard]] constexpr Vec2 lerp(const Vec2& start, const Vec2& end, float32 amount)
{
    return start + (end - start) * amount;
}

[[nodiscard]] constexpr Vec3 lerp(const Vec3& start, const Vec3& end, float32 amount)
{
    return start + (end - start) * amount;
}

[[nodiscard]] constexpr Vec4 lerp(const Vec4& start, const Vec4& end, float32 amount)
{
    return start + (end - start) * amount;
}

[[nodiscard]] inline float32 inverseLerp(float32 start, float32 end, float32 value)
{
    if (nearlyEqual(start, end))
    {
        return 0.0f;
    }

    return (value - start) / (end - start);
}

[[nodiscard]] inline float32 remap(float32 value, float32 inputMin, float32 inputMax,
                                   float32 outputMin, float32 outputMax)
{
    return lerp(outputMin, outputMax, inverseLerp(inputMin, inputMax, value));
}

[[nodiscard]] constexpr float32 step(float32 edge, float32 value)
{
    return value < edge ? 0.0f : 1.0f;
}

[[nodiscard]] inline float32 smoothStep(float32 edge0, float32 edge1, float32 value)
{
    if (nearlyEqual(edge0, edge1))
    {
        return value < edge1 ? 0.0f : 1.0f;
    }

    const float32 amount = saturate((value - edge0) / (edge1 - edge0));
    return amount * amount * (3.0f - 2.0f * amount);
}

[[nodiscard]] inline float32 smootherStep(float32 edge0, float32 edge1, float32 value)
{
    if (nearlyEqual(edge0, edge1))
    {
        return value < edge1 ? 0.0f : 1.0f;
    }

    const float32 amount = saturate((value - edge0) / (edge1 - edge0));
    return amount * amount * amount * (amount * (amount * 6.0f - 15.0f) + 10.0f);
}

[[nodiscard]] inline float32 moveTowards(float32 current, float32 target, float32 maxDelta)
{
    const float32 difference = target - current;

    if (absValue(difference) <= maxDelta)
    {
        return target;
    }

    return current + sign(difference) * maxDelta;
}

[[nodiscard]] inline Vec2 moveTowards(const Vec2& current, const Vec2& target, float32 maxDelta)
{
    const Vec2 difference = target - current;
    const float32 distance = difference.length();

    if (distance <= maxDelta || distance <= constants::kEpsilon)
    {
        return target;
    }

    return current + difference * (maxDelta / distance);
}

[[nodiscard]] inline Vec3 moveTowards(const Vec3& current, const Vec3& target, float32 maxDelta)
{
    const Vec3 difference = target - current;
    const float32 distance = difference.length();

    if (distance <= maxDelta || distance <= constants::kEpsilon)
    {
        return target;
    }

    return current + difference * (maxDelta / distance);
}

}
