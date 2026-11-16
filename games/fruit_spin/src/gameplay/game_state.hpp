#pragma once

#include <array>
#include <cstddef>
#include <vector>

#include "core/math/common/random.hpp"
#include "gameplay/fruit_symbols.hpp"

namespace fruit_spin
{

class GameState
{
public:
    static constexpr std::size_t kReelCount = 3;
    static constexpr int kRowCount = 3;

    GameState();

    void reset();
    void spin();
    void update(float deltaTime);

    [[nodiscard]] int score() const noexcept;
    [[nodiscard]] bool isSpinning() const noexcept;
    [[nodiscard]] bool hasWin() const noexcept;
    [[nodiscard]] bool isWinningCell(std::size_t reel, int row) const noexcept;
    [[nodiscard]] int visibleSymbol(std::size_t reel, int row) const noexcept;
    [[nodiscard]] float reelPosition(std::size_t reel) const noexcept;
    [[nodiscard]] std::size_t stripSize(std::size_t reel) const noexcept;

private:
    struct Reel
    {
        std::vector<FruitSymbol> strip;
        float position = 0.0f;
        float startPosition = 0.0f;
        float targetPosition = 0.0f;
        float delay = 0.0f;
        float duration = 1.6f;
    };

    void buildReels();
    void shuffleStrip(std::vector<FruitSymbol>& strip);
    void evaluate();

    std::array<Reel, kReelCount> m_reels;
    std::array<std::array<bool, kRowCount>, kReelCount> m_winningCells{};
    spy::math::Random m_random;
    int m_score = 0;
    float m_spinElapsed = 0.0f;
    bool m_spinning = false;
    bool m_evaluated = false;
};

}
