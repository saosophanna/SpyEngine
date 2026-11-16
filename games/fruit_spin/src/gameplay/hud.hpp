#pragma once

#include <string_view>

#include "core/math/vector/vec2.hpp"
#include "core/math/vector/vec4.hpp"

namespace spy::renderer
{
class SpriteRenderer;
}

namespace fruit_spin
{

void drawSolidQuad(spy::renderer::SpriteRenderer& renderer,
                   std::string_view texture,
                   const spy::math::Vec2& position,
                   const spy::math::Vec2& size,
                   const spy::math::Vec4& color);

void drawNumber(spy::renderer::SpriteRenderer& renderer,
                std::string_view texture,
                int value,
                const spy::math::Vec2& position,
                float digitWidth,
                float digitHeight,
                const spy::math::Vec4& color);

[[nodiscard]] float numberWidth(int value, float digitWidth);

}
