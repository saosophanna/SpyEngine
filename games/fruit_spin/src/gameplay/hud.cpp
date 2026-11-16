#include "gameplay/hud.hpp"

#include <array>

#include "renderer/sprites/sprite_renderer.hpp"

namespace fruit_spin
{

namespace
{

constexpr std::array<unsigned int, 10> kSegmentMasks = {
    63, 6, 91, 79, 102, 109, 125, 7, 127, 111,
};

constexpr unsigned int kSegmentTop = 1u << 0;
constexpr unsigned int kSegmentTopRight = 1u << 1;
constexpr unsigned int kSegmentBottomRight = 1u << 2;
constexpr unsigned int kSegmentBottom = 1u << 3;
constexpr unsigned int kSegmentBottomLeft = 1u << 4;
constexpr unsigned int kSegmentTopLeft = 1u << 5;
constexpr unsigned int kSegmentMiddle = 1u << 6;

void drawDigit(spy::renderer::SpriteRenderer& renderer,
               std::string_view texture,
               int digit,
               const spy::math::Vec2& position,
               float width,
               float height,
               const spy::math::Vec4& color)
{
    if (digit < 0 || digit > 9)
    {
        return;
    }

    const unsigned int mask = kSegmentMasks[static_cast<std::size_t>(digit)];
    const float thickness = width * 0.2f;
    const float halfHeight = height * 0.5f;
    const float armHeight = halfHeight - thickness * 1.5f;

    if ((mask & kSegmentTop) != 0u)
    {
        drawSolidQuad(renderer, texture,
                      spy::math::Vec2{position.x + thickness, position.y},
                      spy::math::Vec2{width - thickness * 2.0f, thickness}, color);
    }

    if ((mask & kSegmentMiddle) != 0u)
    {
        drawSolidQuad(renderer, texture,
                      spy::math::Vec2{position.x + thickness,
                                      position.y + halfHeight - thickness * 0.5f},
                      spy::math::Vec2{width - thickness * 2.0f, thickness}, color);
    }

    if ((mask & kSegmentBottom) != 0u)
    {
        drawSolidQuad(renderer, texture,
                      spy::math::Vec2{position.x + thickness, position.y + height - thickness},
                      spy::math::Vec2{width - thickness * 2.0f, thickness}, color);
    }

    if ((mask & kSegmentTopLeft) != 0u)
    {
        drawSolidQuad(renderer, texture,
                      spy::math::Vec2{position.x, position.y + thickness},
                      spy::math::Vec2{thickness, armHeight}, color);
    }

    if ((mask & kSegmentTopRight) != 0u)
    {
        drawSolidQuad(renderer, texture,
                      spy::math::Vec2{position.x + width - thickness, position.y + thickness},
                      spy::math::Vec2{thickness, armHeight}, color);
    }

    if ((mask & kSegmentBottomLeft) != 0u)
    {
        drawSolidQuad(renderer, texture,
                      spy::math::Vec2{position.x, position.y + halfHeight + thickness * 0.5f},
                      spy::math::Vec2{thickness, armHeight}, color);
    }

    if ((mask & kSegmentBottomRight) != 0u)
    {
        drawSolidQuad(renderer, texture,
                      spy::math::Vec2{position.x + width - thickness,
                                 position.y + halfHeight + thickness * 0.5f},
                      spy::math::Vec2{thickness, armHeight}, color);
    }
}

int digitCount(int value)
{
    int count = 1;

    while (value >= 10)
    {
        value /= 10;
        ++count;
    }

    return count;
}

}

void drawSolidQuad(spy::renderer::SpriteRenderer& renderer,
                   std::string_view texture,
                   const spy::math::Vec2& position,
                   const spy::math::Vec2& size,
                   const spy::math::Vec4& color)
{
    spy::renderer::SpriteQuad quad{};
    quad.position = position;
    quad.size = size;
    quad.uv = spy::math::Vec4{0.0f, 0.0f, 1.0f, 1.0f};
    quad.color = color;
    renderer.draw(texture, quad);
}

void drawNumber(spy::renderer::SpriteRenderer& renderer,
                std::string_view texture,
                int value,
                const spy::math::Vec2& position,
                float digitWidth,
                float digitHeight,
                const spy::math::Vec4& color)
{
    const int clamped = value < 0 ? 0 : value;
    const int count = digitCount(clamped);
    const float gap = digitWidth * 0.22f;

    int divisor = 1;

    for (int index = 1; index < count; ++index)
    {
        divisor *= 10;
    }

    float x = position.x;
    int remaining = clamped;

    for (int index = 0; index < count; ++index)
    {
        const int digit = remaining / divisor;
        remaining %= divisor;
        divisor = divisor > 1 ? divisor / 10 : 1;

        drawDigit(renderer, texture, digit, spy::math::Vec2{x, position.y}, digitWidth, digitHeight,
                  color);
        x += digitWidth + gap;
    }
}

float numberWidth(int value, float digitWidth)
{
    const int clamped = value < 0 ? 0 : value;
    const int count = digitCount(clamped);
    return static_cast<float>(count) * digitWidth +
           static_cast<float>(count - 1) * digitWidth * 0.22f;
}

}
