#pragma once

#include <cmath>

#include "core/math/common/constants.hpp"
#include "core/math/common/math_utils.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::math {

[[nodiscard]] constexpr float32 linear(float32 amount)
{
    return amount;
}

[[nodiscard]] constexpr float32 easeInQuad(float32 amount)
{
    const float32 value = saturate(amount);
    return value * value;
}

[[nodiscard]] constexpr float32 easeOutQuad(float32 amount)
{
    const float32 value = saturate(amount);
    return value * (2.0f - value);
}

[[nodiscard]] constexpr float32 easeInOutQuad(float32 amount)
{
    const float32 value = saturate(amount);

    if (value < 0.5f)
    {
        return 2.0f * value * value;
    }

    return -1.0f + (4.0f - 2.0f * value) * value;
}

[[nodiscard]] constexpr float32 easeInCubic(float32 amount)
{
    const float32 value = saturate(amount);
    return value * value * value;
}

[[nodiscard]] constexpr float32 easeOutCubic(float32 amount)
{
    const float32 value = saturate(amount) - 1.0f;
    return value * value * value + 1.0f;
}

[[nodiscard]] constexpr float32 easeInOutCubic(float32 amount)
{
    const float32 value = saturate(amount);

    if (value < 0.5f)
    {
        return 4.0f * value * value * value;
    }

    const float32 shifted = 2.0f * value - 2.0f;
    return 0.5f * shifted * shifted * shifted + 1.0f;
}

[[nodiscard]] inline float32 easeInSine(float32 amount)
{
    const float32 value = saturate(amount);
    return 1.0f - std::cos(value * constants::kHalfPi);
}

[[nodiscard]] inline float32 easeOutSine(float32 amount)
{
    const float32 value = saturate(amount);
    return std::sin(value * constants::kHalfPi);
}

[[nodiscard]] inline float32 easeInOutSine(float32 amount)
{
    const float32 value = saturate(amount);
    return -(std::cos(constants::kPi * value) - 1.0f) * 0.5f;
}

[[nodiscard]] inline float32 easeInExpo(float32 amount)
{
    const float32 value = saturate(amount);

    if (value <= 0.0f)
    {
        return 0.0f;
    }

    return std::pow(2.0f, 10.0f * value - 10.0f);
}

[[nodiscard]] inline float32 easeOutExpo(float32 amount)
{
    const float32 value = saturate(amount);

    if (value >= 1.0f)
    {
        return 1.0f;
    }

    return 1.0f - std::pow(2.0f, -10.0f * value);
}

[[nodiscard]] inline float32 easeInOutExpo(float32 amount)
{
    const float32 value = saturate(amount);

    if (value <= 0.0f)
    {
        return 0.0f;
    }

    if (value >= 1.0f)
    {
        return 1.0f;
    }

    if (value < 0.5f)
    {
        return 0.5f * std::pow(2.0f, 20.0f * value - 10.0f);
    }

    return 1.0f - 0.5f * std::pow(2.0f, -20.0f * value + 10.0f);
}

[[nodiscard]] constexpr float32 easeInBack(float32 amount)
{
    constexpr float32 kOvershoot = 1.70158f;
    constexpr float32 kAdjusted = kOvershoot + 1.0f;
    const float32 value = saturate(amount);
    return kAdjusted * value * value * value - kOvershoot * value * value;
}

[[nodiscard]] constexpr float32 easeOutBack(float32 amount)
{
    constexpr float32 kOvershoot = 1.70158f;
    constexpr float32 kAdjusted = kOvershoot + 1.0f;
    const float32 value = saturate(amount) - 1.0f;
    return 1.0f + kAdjusted * value * value * value + kOvershoot * value * value;
}

[[nodiscard]] constexpr float32 easeInOutBack(float32 amount)
{
    constexpr float32 kOvershoot = 1.70158f;
    constexpr float32 kAdjusted = kOvershoot * 1.525f;
    const float32 value = saturate(amount);

    if (value < 0.5f)
    {
        const float32 doubled = 2.0f * value;
        return 0.5f * doubled * doubled * ((kAdjusted + 1.0f) * doubled - kAdjusted);
    }

    const float32 shifted = 2.0f * value - 2.0f;
    return 0.5f * (shifted * shifted * ((kAdjusted + 1.0f) * shifted + kAdjusted) + 2.0f);
}

}
