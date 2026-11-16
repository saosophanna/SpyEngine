#include "gameplay/game_state.hpp"

#include <algorithm>
#include <cmath>

namespace fruit_spin
{

namespace
{

constexpr std::array<std::array<FruitSymbol, 5>, GameState::kReelCount> kReelPalettes = {{
    {FruitSymbol::Cherry, FruitSymbol::Strawberry, FruitSymbol::Orange, FruitSymbol::Grape,
     FruitSymbol::Watermelon},
    {FruitSymbol::Cherry, FruitSymbol::Orange, FruitSymbol::Lemon, FruitSymbol::Apple,
     FruitSymbol::Banana},
    {FruitSymbol::Cherry, FruitSymbol::Orange, FruitSymbol::Watermelon, FruitSymbol::Grape,
     FruitSymbol::Pineapple},
}};

constexpr int kStripRepeats = 3;

constexpr std::array<std::array<int, 3>, 5> kWinLines = {{
    {0, 0, 0},
    {1, 1, 1},
    {2, 2, 2},
    {0, 1, 2},
    {2, 1, 0},
}};

}

GameState::GameState()
{
    m_random.seed(0x5eed1234u);
    buildReels();
}

void GameState::reset()
{
    m_score = 0;

    for (auto& row : m_winningCells)
    {
        row.fill(false);
    }

    for (Reel& reel : m_reels)
    {
        reel.position = 0.0f;
    }

    m_spinElapsed = 0.0f;
    m_spinning = false;
    m_evaluated = false;
}

void GameState::spin()
{
    if (m_spinning)
    {
        return;
    }

    for (auto& row : m_winningCells)
    {
        row.fill(false);
    }

    m_spinning = true;
    m_evaluated = false;
    m_spinElapsed = 0.0f;

    for (std::size_t index = 0; index < m_reels.size(); ++index)
    {
        Reel& reel = m_reels[index];
        const int stripSize = static_cast<int>(reel.strip.size());
        const float start = std::round(reel.position);
        const int extraTurns = 3 + static_cast<int>(index);
        const std::uint32_t lastSymbol = static_cast<std::uint32_t>(stripSize - 1);
        const int landing = static_cast<int>(m_random.rangeUint(0, lastSymbol));

        reel.startPosition = start;
        reel.targetPosition = start + static_cast<float>(extraTurns * stripSize + landing);
        reel.delay = 0.08f * static_cast<float>(index);
        reel.duration = 1.55f + 0.18f * static_cast<float>(index);
    }
}

void GameState::update(float deltaTime)
{
    if (!m_spinning)
    {
        return;
    }

    m_spinElapsed += std::max(deltaTime, 0.0f);
    bool allStopped = true;

    for (Reel& reel : m_reels)
    {
        const float localTime = m_spinElapsed - reel.delay;

        if (localTime <= 0.0f)
        {
            reel.position = reel.startPosition;
            allStopped = false;
            continue;
        }

        const float progress = localTime / reel.duration;

        if (progress >= 1.0f)
        {
            reel.position = reel.targetPosition;
            continue;
        }

        const float eased = 1.0f - std::pow(1.0f - progress, 3.0f);
        reel.position =
            reel.startPosition + (reel.targetPosition - reel.startPosition) * eased;
        allStopped = false;
    }

    if (allStopped)
    {
        m_spinning = false;

        if (!m_evaluated)
        {
            evaluate();
            m_evaluated = true;
        }
    }
}

int GameState::score() const noexcept
{
    return m_score;
}

bool GameState::isSpinning() const noexcept
{
    return m_spinning;
}

bool GameState::hasWin() const noexcept
{
    for (const auto& row : m_winningCells)
    {
        for (const bool cell : row)
        {
            if (cell)
            {
                return true;
            }
        }
    }

    return false;
}

bool GameState::isWinningCell(std::size_t reel, int row) const noexcept
{
    if (reel >= kReelCount || row < 0 || row >= kRowCount)
    {
        return false;
    }

    return m_winningCells[reel][static_cast<std::size_t>(row)];
}

int GameState::visibleSymbol(std::size_t reel, int row) const noexcept
{
    if (reel >= kReelCount)
    {
        return 0;
    }

    const Reel& state = m_reels[reel];
    const int stripSize = static_cast<int>(state.strip.size());

    if (stripSize == 0)
    {
        return 0;
    }

    int index = static_cast<int>(std::floor(state.position)) + row;
    index %= stripSize;

    if (index < 0)
    {
        index += stripSize;
    }

    return static_cast<int>(state.strip[static_cast<std::size_t>(index)]);
}

float GameState::reelPosition(std::size_t reel) const noexcept
{
    if (reel >= kReelCount)
    {
        return 0.0f;
    }

    return m_reels[reel].position;
}

std::size_t GameState::stripSize(std::size_t reel) const noexcept
{
    if (reel >= kReelCount)
    {
        return 0;
    }

    return m_reels[reel].strip.size();
}

void GameState::buildReels()
{
    for (std::size_t reelIndex = 0; reelIndex < m_reels.size(); ++reelIndex)
    {
        const auto& palette = kReelPalettes[reelIndex];
        std::vector<FruitSymbol>& strip = m_reels[reelIndex].strip;
        strip.clear();
        strip.reserve(palette.size() * kStripRepeats);

        for (int repeat = 0; repeat < kStripRepeats; ++repeat)
        {
            for (const FruitSymbol symbol : palette)
            {
                strip.push_back(symbol);
            }
        }

        shuffleStrip(strip);
    }
}

void GameState::shuffleStrip(std::vector<FruitSymbol>& strip)
{
    for (std::size_t index = strip.size(); index > 1; --index)
    {
        const std::size_t other = m_random.index(index);
        std::swap(strip[index - 1], strip[other]);
    }
}

void GameState::evaluate()
{
    for (const auto& line : kWinLines)
    {
        const int first = visibleSymbol(0, line[0]);
        const int second = visibleSymbol(1, line[1]);
        const int third = visibleSymbol(2, line[2]);

        if (first != second || second != third)
        {
            continue;
        }

        for (std::size_t reel = 0; reel < kReelCount; ++reel)
        {
            const int row = line[reel];
            m_winningCells[reel][static_cast<std::size_t>(row)] = true;
        }

        m_score += fruitPayout(static_cast<FruitSymbol>(first)) * 10;
    }
}

}
