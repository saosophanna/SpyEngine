#pragma once

#include "core/math/vector/vec2.hpp"

namespace fruit_spin
{

struct Sparkle
{
    spy::math::Vec2 position{};
    spy::math::Vec2 velocity{};
    float age = 0.0f;
    float lifetime = 0.8f;
    float size = 80.0f;
    float rotation = 0.0f;
};

}
