#pragma once

#include <cmath>
#include <cstdint>

#include "core/math/common/constants.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::math {

[[nodiscard]] constexpr float32 clamp(float32 value, float32 low, float32 high)
{
    return value < low ? low : (value > high ? high : value);
}

[[nodiscard]] constexpr float32 minValue(float32 first, float32 second)
{
    return first < second ? first : second;
}

[[nodiscard]] constexpr float32 maxValue(float32 first, float32 second)
{
    return first > second ? first : second;
}

[[nodiscard]] constexpr float32 absValue(float32 value)
{
    return value < 0.0f ? -value : value;
}

[[nodiscard]] constexpr float32 sign(float32 value)
{
    return value > 0.0f ? 1.0f : (value < 0.0f ? -1.0f : 0.0f);
}

[[nodiscard]] constexpr float32 saturate(float32 value)
{
    return clamp(value, 0.0f, 1.0f);
}

[[nodiscard]] constexpr float32 square(float32 value)
{
    return value * value;
}

[[nodiscard]] inline float32 wrap(float32 value, float32 low, float32 high)
{
    const float32 range = high - low;

    if (range <= 0.0f)
    {
        return low;
    }

    float32 result = std::fmod(value - low, range);

    if (result < 0.0f)
    {
        result += range;
    }

    return low + result;
}

[[nodiscard]] inline bool nearlyEqual(float32 first, float32 second,
                                      float32 absoluteTolerance = constants::kEpsilon,
                                      float32 relativeTolerance = constants::kEpsilon)
{
    if (first == second)
    {
        return true;
    }

    const float32 difference = std::fabs(first - second);

    if (difference <= absoluteTolerance)
    {
        return true;
    }

    const float32 magnitude = std::fmax(std::fabs(first), std::fabs(second));
    return difference <= relativeTolerance * magnitude;
}

[[nodiscard]] constexpr float32 degreesToRadians(float32 degrees)
{
    return degrees * constants::kDegToRad;
}

[[nodiscard]] constexpr float32 radiansToDegrees(float32 radians)
{
    return radians * constants::kRadToDeg;
}

[[nodiscard]] constexpr bool isPowerOfTwo(uint32 value)
{
    return value != 0 && (value & (value - 1)) == 0;
}

[[nodiscard]] constexpr uint32 nextPowerOfTwo(uint32 value)
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
