#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "core/math/common/random.hpp"
#include "core/math/vector/vec2.hpp"
#include "ecs/ecs.hpp"
#include "gameplay/game_state.hpp"

namespace spy::platform
{
class Window;
}

namespace spy::renderer
{
class SpriteRenderer;
}

namespace fruit_spin
{

class GameScene
{
public:
    struct Layout
    {
        float scale = 1.0f;
        float offsetX = 0.0f;
        float offsetY = 0.0f;
    };

    GameScene() = default;

    [[nodiscard]] bool initialize(spy::renderer::SpriteRenderer& renderer,
                                  std::string_view assetsDirectory);
    void update(float deltaTime, const spy::platform::Window& window);
    void render(spy::renderer::SpriteRenderer& renderer, float windowWidth, float windowHeight);

private:
    [[nodiscard]] static Layout layoutFor(float width, float height);

    void spawnWinSparkles();
    void renderBackground(spy::renderer::SpriteRenderer& renderer,
                          float windowWidth,
                          float windowHeight);
    void renderBoard(spy::renderer::SpriteRenderer& renderer, const Layout& layout);
    void renderHeader(spy::renderer::SpriteRenderer& renderer, const Layout& layout);
    void renderButton(spy::renderer::SpriteRenderer& renderer, const Layout& layout);
    void renderSparkles(spy::renderer::SpriteRenderer& renderer, const Layout& layout);

    GameState m_state;
    spy::ecs::Registry m_registry;
    spy::math::Random m_random;
    float m_time = 0.0f;
    bool m_wasSpinning = false;
    bool m_ready = false;
};

}
