#pragma once

#include <limits>

#include "core/math/common/math_utils.hpp"
#include "core/math/vector/vec2.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::math {

struct SmoothDampState
{
    float32 velocity = 0.0f;
};

struct SmoothDampState2D
{
    Vec2 velocity{};
};

[[nodiscard]] inline float32 smoothDampComponent(float32 current, float32 target,
                                                 float32& velocity, float32 smoothTime,
                                                 float32 deltaTime, float32 maxSpeed)
{
    if (deltaTime <= 0.0f)
    {
        return current;
    }

    if (smoothTime <= 0.0f)
    {
        velocity = 0.0f;
        return target;
    }

    const float32 omega = 2.0f / smoothTime;
    const float32 scaledTime = omega * deltaTime;
    const float32 decay = 1.0f / (1.0f + scaledTime + 0.48f * scaledTime * scaledTime +
                                0.235f * scaledTime * scaledTime * scaledTime);

    const float32 originalTarget = target;
    const float32 maximumChange = maxSpeed * smoothTime;
    float32 change = clamp(current - target, -maximumChange, maximumChange);
    target = current - change;

    const float32 impulse = (velocity + omega * change) * deltaTime;
    velocity = (velocity - omega * impulse) * decay;

    float32 output = target + (change + impulse) * decay;

    const bool wasAbove = originalTarget - current > 0.0f;
    const bool crossedTarget = output > originalTarget;

    if (wasAbove == crossedTarget)
    {
        output = originalTarget;
        velocity = 0.0f;
    }

    return output;
}

[[nodiscard]] inline float32 smoothDamp(float32 current, float32 target, SmoothDampState& state,
                                      float32 smoothTime, float32 deltaTime,
                                      float32 maxSpeed = std::numeric_limits<float32>::max())
{
    return smoothDampComponent(current, target, state.velocity, smoothTime, deltaTime, maxSpeed);
}

[[nodiscard]] inline Vec2 smoothDamp(const Vec2& current, const Vec2& target,
                                     SmoothDampState2D& state, float32 smoothTime,
                                     float32 deltaTime,
                                     float32 maxSpeed = std::numeric_limits<float32>::max())
{
    return Vec2{smoothDampComponent(current.x, target.x, state.velocity.x, smoothTime, deltaTime,
                                    maxSpeed),
                smoothDampComponent(current.y, target.y, state.velocity.y, smoothTime, deltaTime,
                                    maxSpeed)};
}

}
