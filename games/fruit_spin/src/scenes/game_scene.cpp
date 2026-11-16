#include "scenes/game_scene.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>

#include "assets/image/png_decoder.hpp"
#include "components/sparkle.hpp"
#include "core/math/vector/vec4.hpp"
#include "gameplay/fruit_symbols.hpp"
#include "gameplay/hud.hpp"
#include "platform/window/window.hpp"
#include "renderer/sprites/sprite_renderer.hpp"
#include "systems/sparkle_system.hpp"

namespace fruit_spin
{

namespace
{

constexpr float kDesignWidth = 1280.0f;
constexpr float kDesignHeight = 720.0f;

constexpr float kCellSize = 150.0f;
constexpr float kCellGap = 12.0f;
constexpr float kPitch = kCellSize + kCellGap;
constexpr float kBoardWidth = static_cast<float>(GameState::kReelCount) * kCellSize +
                              static_cast<float>(GameState::kReelCount - 1) * kCellGap;
constexpr float kBoardHeight = static_cast<float>(GameState::kRowCount) * kCellSize +
                               static_cast<float>(GameState::kRowCount - 1) * kCellGap;
constexpr float kBoardLeft = (kDesignWidth - kBoardWidth) * 0.5f;
constexpr float kBoardTop = 118.0f;
constexpr float kPanelPadding = 18.0f;
constexpr float kPanelLeft = kBoardLeft - kPanelPadding;
constexpr float kPanelTop = kBoardTop - kPanelPadding;
constexpr float kPanelWidth = kBoardWidth + kPanelPadding * 2.0f;
constexpr float kPanelHeight = kBoardHeight + kPanelPadding * 2.0f;

constexpr float kButtonLeft = 510.0f;
constexpr float kButtonTop = 640.0f;
constexpr float kButtonWidth = 260.0f;
constexpr float kButtonHeight = 58.0f;

const spy::math::Vec4 kPanelBorder{0.30f, 0.16f, 0.48f, 1.0f};
const spy::math::Vec4 kPanelFill{0.055f, 0.045f, 0.10f, 1.0f};
const spy::math::Vec4 kSlotFill{0.10f, 0.08f, 0.16f, 1.0f};
const spy::math::Vec4 kSlotInner{0.135f, 0.105f, 0.205f, 1.0f};
const spy::math::Vec4 kGold{1.0f, 0.84f, 0.36f, 1.0f};
const spy::math::Vec4 kScoreColor{1.0f, 0.94f, 0.74f, 1.0f};
const spy::math::Vec4 kWhite{1.0f, 1.0f, 1.0f, 1.0f};

void drawDesignRect(spy::renderer::SpriteRenderer& renderer,
                    const GameScene::Layout& layout,
                    float x,
                    float y,
                    float width,
                    float height,
                    const spy::math::Vec4& color)
{
    const spy::math::Vec2 position{layout.offsetX + x * layout.scale,
                              layout.offsetY + y * layout.scale};
    const spy::math::Vec2 size{width * layout.scale, height * layout.scale};
    drawSolidQuad(renderer, "white", position, size, color);
}

void drawDesignSprite(spy::renderer::SpriteRenderer& renderer,
                      const GameScene::Layout& layout,
                      std::string_view texture,
                      float x,
                      float y,
                      float width,
                      float height,
                      const UvRect& uv,
                      const spy::math::Vec4& color,
                      float rotation = 0.0f)
{
    spy::renderer::SpriteQuad quad{};
    quad.position = spy::math::Vec2{layout.offsetX + x * layout.scale,
                               layout.offsetY + y * layout.scale};
    quad.size = spy::math::Vec2{width * layout.scale, height * layout.scale};
    quad.uv = spy::math::Vec4{uv.minU, uv.minV, uv.maxU, uv.maxV};
    quad.color = color;
    quad.rotation = rotation;
    renderer.draw(texture, quad);
}

void drawDesignBar(spy::renderer::SpriteRenderer& renderer,
                   const GameScene::Layout& layout,
                   float centerX,
                   float centerY,
                   float length,
                   float thickness,
                   float rotation,
                   const spy::math::Vec4& color)
{
    drawDesignSprite(renderer,
                     layout,
                     "white",
                     centerX - length * 0.5f,
                     centerY - thickness * 0.5f,
                     length,
                     thickness,
                     UvRect{},
                     color,
                     rotation);
}

}

bool GameScene::initialize(spy::renderer::SpriteRenderer& renderer,
                           std::string_view assetsDirectory)
{
    const std::filesystem::path root(assetsDirectory);

    const spy::assets::Image fruitSheet =
        spy::assets::decodePngFile((root / "textures" / "fruit" / "fruit_sheet.png").string());
    const spy::assets::Image sparkleSheet = spy::assets::decodePngFile(
        (root / "textures" / "fx" / "sparkle" / "sparkle_sheet.png").string());

    if (!fruitSheet.valid() || !sparkleSheet.valid())
    {
        return false;
    }

    const std::uint8_t whitePixel[4] = {255, 255, 255, 255};

    if (!renderer.createTexture("white", 1, 1, whitePixel) ||
        !renderer.createTexture("fruit", fruitSheet.width, fruitSheet.height,
                                fruitSheet.pixels.data()) ||
        !renderer.createTexture("sparkle", sparkleSheet.width, sparkleSheet.height,
                                sparkleSheet.pixels.data()))
    {
        return false;
    }

    m_random.seed(0xf00d5678u);
    m_ready = true;
    return true;
}

void GameScene::update(float deltaTime, const spy::platform::Window& window)
{
    if (!m_ready)
    {
        return;
    }

    m_time += deltaTime;
    m_state.update(deltaTime);
    updateSparkles(m_registry, deltaTime);

    if (m_state.isSpinning())
    {
        m_wasSpinning = true;
    }
    else if (m_wasSpinning)
    {
        m_wasSpinning = false;
        spawnWinSparkles();
    }

    if (window.wasKeyPressed(spy::platform::Key::R))
    {
        m_state.reset();
    }

    if (m_state.isSpinning() || !window.wasMousePressed())
    {
        return;
    }

    const Layout layout = layoutFor(static_cast<float>(window.width()),
                                    static_cast<float>(window.height()));
    const float mouseX = (window.mouseX() - layout.offsetX) / layout.scale;
    const float mouseY = (window.mouseY() - layout.offsetY) / layout.scale;

    const bool overButton = mouseX >= kButtonLeft && mouseX <= kButtonLeft + kButtonWidth &&
                            mouseY >= kButtonTop && mouseY <= kButtonTop + kButtonHeight;

    if (overButton)
    {
        m_state.spin();
    }
}

void GameScene::render(spy::renderer::SpriteRenderer& renderer,
                       float windowWidth,
                       float windowHeight)
{
    if (!m_ready)
    {
        return;
    }

    const Layout layout = layoutFor(windowWidth, windowHeight);

    renderBackground(renderer, windowWidth, windowHeight);
    renderBoard(renderer, layout);
    renderSparkles(renderer, layout);
    renderHeader(renderer, layout);
    renderButton(renderer, layout);
}

GameScene::Layout GameScene::layoutFor(float width, float height)
{
    if (width <= 0.0f || height <= 0.0f)
    {
        return Layout{1.0f, 0.0f, 0.0f};
    }

    const float scale = std::min(width / kDesignWidth, height / kDesignHeight);

    return Layout{scale,
                  (width - kDesignWidth * scale) * 0.5f,
                  (height - kDesignHeight * scale) * 0.5f};
}

void GameScene::spawnWinSparkles()
{
    for (std::size_t reel = 0; reel < GameState::kReelCount; ++reel)
    {
        for (int row = 0; row < GameState::kRowCount; ++row)
        {
            if (!m_state.isWinningCell(reel, row))
            {
                continue;
            }

            const spy::math::Vec2 center{
                kBoardLeft + static_cast<float>(reel) * kPitch + kCellSize * 0.5f,
                kBoardTop + static_cast<float>(row) * kPitch + kCellSize * 0.5f};

            for (int index = 0; index < 4; ++index)
            {
                const float spread = kCellSize * 0.32f;
                const spy::math::Vec2 offset{m_random.rangeFloat(-spread, spread),
                                             m_random.rangeFloat(-spread, spread)};
                const spy::math::Vec2 velocity{m_random.rangeFloat(-55.0f, 55.0f),
                                               m_random.rangeFloat(-155.0f, -45.0f)};

                (void)spawnSparkle(m_registry,
                                   center + offset,
                                   velocity,
                                   m_random.rangeFloat(0.55f, 1.05f),
                                   m_random.rangeFloat(70.0f, 135.0f));
            }
        }
    }
}

void GameScene::renderBackground(spy::renderer::SpriteRenderer& renderer,
                                 float windowWidth,
                                 float windowHeight)
{
    constexpr int kBandCount = 48;
    const spy::math::Vec4 top{0.115f, 0.055f, 0.205f, 1.0f};
    const spy::math::Vec4 bottom{0.025f, 0.018f, 0.055f, 1.0f};
    const float bandHeight = windowHeight / static_cast<float>(kBandCount);

    for (int band = 0; band < kBandCount; ++band)
    {
        const float t = static_cast<float>(band) / static_cast<float>(kBandCount - 1);
        const spy::math::Vec4 color{top.x + (bottom.x - top.x) * t,
                               top.y + (bottom.y - top.y) * t,
                               top.z + (bottom.z - top.z) * t,
                               1.0f};

        spy::renderer::SpriteQuad quad{};
        quad.position = spy::math::Vec2{0.0f, static_cast<float>(band) * bandHeight};
        quad.size = spy::math::Vec2{windowWidth, bandHeight + 1.0f};
        quad.color = color;
        renderer.draw("white", quad);
    }
}

void GameScene::renderBoard(spy::renderer::SpriteRenderer& renderer, const Layout& layout)
{
    for (int ring = 6; ring >= 1; --ring)
    {
        const float inset = static_cast<float>(ring) * 14.0f;
        const spy::math::Vec4 glow{0.56f, 0.32f, 0.92f, 0.018f * static_cast<float>(7 - ring)};
        drawDesignRect(renderer, layout, kPanelLeft - inset, kPanelTop - inset,
                       kPanelWidth + inset * 2.0f, kPanelHeight + inset * 2.0f, glow);
    }

    drawDesignRect(renderer, layout, kPanelLeft - 4.0f, kPanelTop - 4.0f,
                   kPanelWidth + 8.0f, kPanelHeight + 8.0f, kPanelBorder);
    drawDesignRect(renderer, layout, kPanelLeft, kPanelTop, kPanelWidth, kPanelHeight, kPanelFill);

    for (std::size_t reel = 0; reel < GameState::kReelCount; ++reel)
    {
        for (int row = 0; row < GameState::kRowCount; ++row)
        {
            const float x = kBoardLeft + static_cast<float>(reel) * kPitch;
            const float y = kBoardTop + static_cast<float>(row) * kPitch;

            drawDesignRect(renderer, layout, x, y, kCellSize, kCellSize, kSlotFill);
            drawDesignRect(renderer, layout, x + 3.0f, y + 3.0f, kCellSize - 6.0f,
                           kCellSize - 6.0f, kSlotInner);
        }
    }

    renderer.setClip(layout.offsetX + kBoardLeft * layout.scale,
                     layout.offsetY + kBoardTop * layout.scale,
                     kBoardWidth * layout.scale,
                     kBoardHeight * layout.scale);

    for (std::size_t reel = 0; reel < GameState::kReelCount; ++reel)
    {
        const float position = m_state.reelPosition(reel);
        const float base = std::floor(position);
        const float fraction = position - base;

        for (int row = -1; row <= GameState::kRowCount; ++row)
        {
            const int symbol = m_state.visibleSymbol(reel, row);
            const UvRect uv = fruitUv(static_cast<FruitSymbol>(symbol));
            const float x = kBoardLeft + static_cast<float>(reel) * kPitch;
            const float y = kBoardTop + (static_cast<float>(row) - fraction) * kPitch;

            drawDesignSprite(renderer, layout, "fruit", x, y, kCellSize, kCellSize, uv, kWhite);
        }
    }

    renderer.clearClip();

    drawDesignRect(renderer, layout, kPanelLeft, kPanelTop, kPanelWidth,
                   kBoardTop - kPanelTop, kPanelFill);
    drawDesignRect(renderer, layout, kPanelLeft, kBoardTop + kBoardHeight, kPanelWidth,
                   kPanelTop + kPanelHeight - (kBoardTop + kBoardHeight), kPanelFill);
    drawDesignRect(renderer, layout, kPanelLeft, kBoardTop, kBoardLeft - kPanelLeft,
                   kBoardHeight, kPanelFill);
    drawDesignRect(renderer, layout, kBoardLeft + kBoardWidth, kBoardTop,
                   kPanelLeft + kPanelWidth - (kBoardLeft + kBoardWidth), kBoardHeight, kPanelFill);

    for (int row = 1; row < GameState::kRowCount; ++row)
    {
        drawDesignRect(renderer, layout, kBoardLeft,
                       kBoardTop + static_cast<float>(row) * kPitch - kCellGap,
                       kBoardWidth, kCellGap, kPanelFill);
    }

    for (std::size_t reel = 1; reel < GameState::kReelCount; ++reel)
    {
        drawDesignRect(renderer, layout,
                       kBoardLeft + static_cast<float>(reel) * kPitch - kCellGap,
                       kBoardTop, kCellGap, kBoardHeight, kPanelFill);
    }

    if (!m_state.hasWin())
    {
        return;
    }

    const float pulse = 0.55f + 0.45f * std::sin(m_time * 8.0f);

    for (std::size_t reel = 0; reel < GameState::kReelCount; ++reel)
    {
        for (int row = 0; row < GameState::kRowCount; ++row)
        {
            if (!m_state.isWinningCell(reel, row))
            {
                continue;
            }

            const spy::math::Vec4 outline{kGold.x, kGold.y, kGold.z, pulse};
            const float x = kBoardLeft + static_cast<float>(reel) * kPitch;
            const float y = kBoardTop + static_cast<float>(row) * kPitch;
            constexpr float kOutline = 4.0f;

            drawDesignRect(renderer, layout, x + 2.0f, y + 2.0f, kCellSize - 4.0f, kOutline,
                           outline);
            drawDesignRect(renderer, layout, x + 2.0f, y + kCellSize - 6.0f,
                           kCellSize - 4.0f, kOutline, outline);
            drawDesignRect(renderer, layout, x + 2.0f, y + 2.0f, kOutline,
                           kCellSize - 4.0f, outline);
            drawDesignRect(renderer, layout, x + kCellSize - 6.0f, y + 2.0f, kOutline,
                           kCellSize - 4.0f, outline);
        }
    }
}

void GameScene::renderSparkles(spy::renderer::SpriteRenderer& renderer, const Layout& layout)
{
    m_registry.each<Sparkle>(
        [&](spy::ecs::Entity, Sparkle& sparkle)
        {
            const float progress = std::clamp(sparkle.age / sparkle.lifetime, 0.0f, 1.0f);
            const std::uint32_t frame =
                static_cast<std::uint32_t>(progress * static_cast<float>(kSparkleFrameCount - 1));
            const UvRect uv = sparkleUv(frame);
            const spy::math::Vec4 color{1.0f, 0.90f, 0.62f, 1.0f - progress};

            drawDesignSprite(renderer, layout, "sparkle",
                             sparkle.position.x - sparkle.size * 0.5f,
                             sparkle.position.y - sparkle.size * 0.5f,
                             sparkle.size, sparkle.size, uv, color, sparkle.rotation);
        });
}

void GameScene::renderHeader(spy::renderer::SpriteRenderer& renderer, const Layout& layout)
{
    drawDesignRect(renderer, layout, 40.0f, 22.0f, 76.0f, 76.0f, kPanelBorder);
    drawDesignRect(renderer, layout, 44.0f, 26.0f, 68.0f, 68.0f, kPanelFill);
    drawDesignSprite(renderer, layout, "fruit", 51.0f, 33.0f, 54.0f, 54.0f,
                     fruitUv(FruitSymbol::Apple), kWhite);

    drawDesignRect(renderer, layout, 940.0f, 22.0f, 292.0f, 76.0f, kPanelBorder);
    drawDesignRect(renderer, layout, 944.0f, 26.0f, 284.0f, 68.0f,
                   spy::math::Vec4{0.07f, 0.05f, 0.11f, 0.96f});

    constexpr float kDigitWidth = 30.0f;
    const float digitsWidth = numberWidth(m_state.score(), kDigitWidth);
    const float digitsX = 940.0f + 292.0f - 22.0f - digitsWidth;

    drawNumber(renderer, "white", m_state.score(), spy::math::Vec2{digitsX, 38.0f}, kDigitWidth,
               46.0f, kScoreColor);
}

void GameScene::renderButton(spy::renderer::SpriteRenderer& renderer, const Layout& layout)
{
    const bool spinning = m_state.isSpinning();
    const float pulse = 0.5f + 0.5f * std::sin(m_time * 4.5f);

    const spy::math::Vec4 glow{1.0f, 0.55f, 0.15f, 0.08f + 0.10f * pulse};
    drawDesignRect(renderer, layout, kButtonLeft - 10.0f, kButtonTop - 10.0f,
                   kButtonWidth + 20.0f, kButtonHeight + 20.0f, glow);

    const spy::math::Vec4 border = spinning ? spy::math::Vec4{0.36f, 0.25f, 0.17f, 1.0f}
                                       : spy::math::Vec4{1.0f, 0.72f, 0.28f, 1.0f};
    const spy::math::Vec4 fill = spinning ? spy::math::Vec4{0.16f, 0.12f, 0.10f, 1.0f}
                                     : spy::math::Vec4{0.93f, 0.46f, 0.10f, 1.0f};

    drawDesignRect(renderer, layout, kButtonLeft, kButtonTop, kButtonWidth, kButtonHeight, border);
    drawDesignRect(renderer, layout, kButtonLeft + 4.0f, kButtonTop + 4.0f,
                   kButtonWidth - 8.0f, kButtonHeight - 8.0f, fill);

    if (!spinning)
    {
        drawDesignRect(renderer, layout, kButtonLeft + 4.0f, kButtonTop + 4.0f,
                       kButtonWidth - 8.0f, 11.0f, spy::math::Vec4{1.0f, 0.82f, 0.42f, 0.32f});
    }

    const spy::math::Vec4 chevronColor = spinning ? spy::math::Vec4{0.52f, 0.46f, 0.40f, 1.0f}
                                            : spy::math::Vec4{1.0f, 0.97f, 0.88f, 1.0f};
    const float centerY = kButtonTop + kButtonHeight * 0.5f;
    constexpr float kRotation = 0.78539816f;

    for (const float centerX : {616.0f, 646.0f})
    {
        drawDesignBar(renderer, layout, centerX, centerY - 14.0f, 48.0f, 12.0f, kRotation,
                      chevronColor);
        drawDesignBar(renderer, layout, centerX, centerY + 14.0f, 48.0f, 12.0f, -kRotation,
                      chevronColor);
    }
}

}
