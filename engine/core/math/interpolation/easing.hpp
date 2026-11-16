#pragma once

#include <cmath>

#include "core/math/common/constants.hpp"
#include "core/math/common/math_utils.hpp"

namespace spy::math {

[[nodiscard]] constexpr float linear(float amount)
{
    return amount;
}

[[nodiscard]] constexpr float easeInQuad(float amount)
{
    const float value = saturate(amount);
    return value * value;
}

[[nodiscard]] constexpr float easeOutQuad(float amount)
{
    const float value = saturate(amount);
    return value * (2.0f - value);
}

[[nodiscard]] constexpr float easeInOutQuad(float amount)
{
    const float value = saturate(amount);

    if (value < 0.5f)
    {
        return 2.0f * value * value;
    }

    return -1.0f + (4.0f - 2.0f * value) * value;
}

[[nodiscard]] constexpr float easeInCubic(float amount)
{
    const float value = saturate(amount);
    return value * value * value;
}

[[nodiscard]] constexpr float easeOutCubic(float amount)
{
    const float value = saturate(amount) - 1.0f;
    return value * value * value + 1.0f;
}

[[nodiscard]] constexpr float easeInOutCubic(float amount)
{
    const float value = saturate(amount);

    if (value < 0.5f)
    {
        return 4.0f * value * value * value;
    }

    const float shifted = 2.0f * value - 2.0f;
    return 0.5f * shifted * shifted * shifted + 1.0f;
}

[[nodiscard]] inline float easeInSine(float amount)
{
    const float value = saturate(amount);
    return 1.0f - std::cos(value * constants::kHalfPi);
}

[[nodiscard]] inline float easeOutSine(float amount)
{
    const float value = saturate(amount);
    return std::sin(value * constants::kHalfPi);
}

[[nodiscard]] inline float easeInOutSine(float amount)
{
    const float value = saturate(amount);
    return -(std::cos(constants::kPi * value) - 1.0f) * 0.5f;
}

[[nodiscard]] inline float easeInExpo(float amount)
{
    const float value = saturate(amount);

    if (value <= 0.0f)
    {
        return 0.0f;
    }

    return std::pow(2.0f, 10.0f * value - 10.0f);
}

[[nodiscard]] inline float easeOutExpo(float amount)
{
    const float value = saturate(amount);

    if (value >= 1.0f)
    {
        return 1.0f;
    }

    return 1.0f - std::pow(2.0f, -10.0f * value);
}

[[nodiscard]] inline float easeInOutExpo(float amount)
{
    const float value = saturate(amount);

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

[[nodiscard]] constexpr float easeInBack(float amount)
{
    constexpr float kOvershoot = 1.70158f;
    constexpr float kAdjusted = kOvershoot + 1.0f;
    const float value = saturate(amount);
    return kAdjusted * value * value * value - kOvershoot * value * value;
}

[[nodiscard]] constexpr float easeOutBack(float amount)
{
    constexpr float kOvershoot = 1.70158f;
    constexpr float kAdjusted = kOvershoot + 1.0f;
    const float value = saturate(amount) - 1.0f;
    return 1.0f + kAdjusted * value * value * value + kOvershoot * value * value;
}

[[nodiscard]] constexpr float easeInOutBack(float amount)
{
    constexpr float kOvershoot = 1.70158f;
    constexpr float kAdjusted = kOvershoot * 1.525f;
    const float value = saturate(amount);

    if (value < 0.5f)
    {
        const float doubled = 2.0f * value;
        return 0.5f * doubled * doubled * ((kAdjusted + 1.0f) * doubled - kAdjusted);
    }

    const float shifted = 2.0f * value - 2.0f;
    return 0.5f * (shifted * shifted * ((kAdjusted + 1.0f) * shifted + kAdjusted) + 2.0f);
}

}
