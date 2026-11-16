#pragma once

#include <cmath>
#include <cstdint>

#include "core/math/common/constants.hpp"

namespace spy::math {

[[nodiscard]] constexpr float clamp(float value, float low, float high)
{
    return value < low ? low : (value > high ? high : value);
}

[[nodiscard]] constexpr float minValue(float first, float second)
{
    return first < second ? first : second;
}

[[nodiscard]] constexpr float maxValue(float first, float second)
{
    return first > second ? first : second;
}

[[nodiscard]] constexpr float absValue(float value)
{
    return value < 0.0f ? -value : value;
}

[[nodiscard]] constexpr float sign(float value)
{
    return value > 0.0f ? 1.0f : (value < 0.0f ? -1.0f : 0.0f);
}

[[nodiscard]] constexpr float saturate(float value)
{
    return clamp(value, 0.0f, 1.0f);
}

[[nodiscard]] constexpr float square(float value)
{
    return value * value;
}

[[nodiscard]] inline float wrap(float value, float low, float high)
{
    const float range = high - low;

    if (range <= 0.0f)
    {
        return low;
    }

    float result = std::fmod(value - low, range);

    if (result < 0.0f)
    {
        result += range;
    }

    return low + result;
}

[[nodiscard]] inline bool nearlyEqual(float first, float second,
                                      float absoluteTolerance = constants::kEpsilon,
                                      float relativeTolerance = constants::kEpsilon)
{
    if (first == second)
    {
        return true;
    }

    const float difference = std::fabs(first - second);

    if (difference <= absoluteTolerance)
    {
        return true;
    }

    const float magnitude = std::fmax(std::fabs(first), std::fabs(second));
    return difference <= relativeTolerance * magnitude;
}

[[nodiscard]] constexpr float degreesToRadians(float degrees)
{
    return degrees * constants::kDegToRad;
}

[[nodiscard]] constexpr float radiansToDegrees(float radians)
{
    return radians * constants::kRadToDeg;
}

[[nodiscard]] constexpr bool isPowerOfTwo(std::uint32_t value)
{
    return value != 0 && (value & (value - 1)) == 0;
}

[[nodiscard]] constexpr std::uint32_t nextPowerOfTwo(std::uint32_t value)
{
    if (value <= 1)
    {
        return 1;
    }

    --value;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    return value + 1;
}

}
