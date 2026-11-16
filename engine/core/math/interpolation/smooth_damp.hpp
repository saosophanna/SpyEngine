#pragma once

#include <limits>

#include "core/math/common/math_utils.hpp"
#include "core/math/vector/vec2.hpp"

namespace spy::math {

struct SmoothDampState
{
    float velocity = 0.0f;
};

struct SmoothDampState2D
{
    Vec2 velocity{};
};

[[nodiscard]] inline float smoothDampComponent(float current, float target, float& velocity,
                                                float smoothTime, float deltaTime, float maxSpeed)
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

    const float omega = 2.0f / smoothTime;
    const float scaledTime = omega * deltaTime;
    const float decay = 1.0f / (1.0f + scaledTime + 0.48f * scaledTime * scaledTime +
                                0.235f * scaledTime * scaledTime * scaledTime);

    const float originalTarget = target;
    const float maximumChange = maxSpeed * smoothTime;
    float change = clamp(current - target, -maximumChange, maximumChange);
    target = current - change;

    const float impulse = (velocity + omega * change) * deltaTime;
    velocity = (velocity - omega * impulse) * decay;

    float output = target + (change + impulse) * decay;

    const bool wasAbove = originalTarget - current > 0.0f;
    const bool crossedTarget = output > originalTarget;

    if (wasAbove == crossedTarget)
    {
        output = originalTarget;
        velocity = 0.0f;
    }

    return output;
}

[[nodiscard]] inline float smoothDamp(float current, float target, SmoothDampState& state,
                                      float smoothTime, float deltaTime,
                                      float maxSpeed = std::numeric_limits<float>::max())
{
    return smoothDampComponent(current, target, state.velocity, smoothTime, deltaTime, maxSpeed);
}

[[nodiscard]] inline Vec2 smoothDamp(const Vec2& current, const Vec2& target,
                                     SmoothDampState2D& state, float smoothTime, float deltaTime,
                                     float maxSpeed = std::numeric_limits<float>::max())
{
    return Vec2{smoothDampComponent(current.x, target.x, state.velocity.x, smoothTime, deltaTime,
                                    maxSpeed),
                smoothDampComponent(current.y, target.y, state.velocity.y, smoothTime, deltaTime,
                                    maxSpeed)};
}

}
