#include <cstdio>

#include "gameplay/fruit_symbols.hpp"
#include "gameplay/game_state.hpp"

namespace {

int g_failures = 0;

void check(bool condition, const char* expression)
{
    if (!condition)
    {
        std::fprintf(stderr, "FAILED: %s\n", expression);
        ++g_failures;
    }
}

bool reelContains(const fruit_spin::GameState& state, std::size_t reel, int symbol)
{
    const std::size_t size = state.stripSize(reel);

    for (std::size_t row = 0; row < size; ++row)
    {
        if (state.visibleSymbol(reel, static_cast<int>(row)) == symbol)
        {
            return true;
        }
    }

    return false;
}

}

int main()
{
    fruit_spin::GameState state;

    for (std::size_t reel = 0; reel < fruit_spin::GameState::kReelCount; ++reel)
    {
        check(state.stripSize(reel) > 0, "every reel has a strip");
    }

    bool sharedSymbol = false;

    for (std::uint32_t symbol = 0; symbol < fruit_spin::kFruitSymbolCount; ++symbol)
    {
        if (reelContains(state, 0, static_cast<int>(symbol)) &&
            reelContains(state, 1, static_cast<int>(symbol)) &&
            reelContains(state, 2, static_cast<int>(symbol)))
        {
            sharedSymbol = true;
            break;
        }
    }

    check(sharedSymbol, "all reels share at least one symbol");

    int spins = 0;
    int wins = 0;

    for (int index = 0; index < 40; ++index)
    {
        state.spin();

        for (int frame = 0; frame < 600 && state.isSpinning(); ++frame)
        {
            state.update(1.0f / 60.0f);
        }

        check(!state.isSpinning(), "spin settles");
        check(state.score() >= 0, "score stays non-negative");

        for (std::size_t reel = 0; reel < fruit_spin::GameState::kReelCount; ++reel)
        {
            for (int row = 0; row < fruit_spin::GameState::kRowCount; ++row)
            {
                if (state.isWinningCell(reel, row))
                {
                    check(state.hasWin(), "winning cells imply a win");
                }

                const int symbol = state.visibleSymbol(reel, row);
                check(symbol >= 0 && symbol < static_cast<int>(fruit_spin::kFruitSymbolCount),
                      "visible symbol is valid");
            }
        }

        ++spins;

        if (state.hasWin())
        {
            ++wins;
        }
    }

    check(spins == 40, "spin loop runs every round");
    check(wins > 0, "wins occur over repeated spins");

    state.reset();
    check(state.score() == 0, "reset clears the score");

    for (std::size_t reel = 0; reel < fruit_spin::GameState::kReelCount; ++reel)
    {
        for (int row = 0; row < fruit_spin::GameState::kRowCount; ++row)
        {
            check(!state.isWinningCell(reel, row), "reset clears winning cells");
        }
    }

    std::printf("%s\n", g_failures == 0 ? "fruit spin logic tests passed"
                                        : "fruit spin logic tests failed");
    return g_failures == 0 ? 0 : 1;
}
