#pragma once

#include <cstdint>

namespace fruit_spin
{

enum class FruitSymbol : std::uint8_t
{
    Cherry = 0,
    Strawberry,
    Watermelon,
    Orange,
    Lemon,
    Grape,
    Apple,
    Banana,
    Pineapple,
    Blueberry,
    Count,
};

constexpr std::uint32_t kFruitSymbolCount = static_cast<std::uint32_t>(FruitSymbol::Count);
constexpr std::uint32_t kFruitSheetColumns = 5;
constexpr std::uint32_t kFruitSheetRows = 2;
constexpr std::uint32_t kSparkleSheetColumns = 4;
constexpr std::uint32_t kSparkleSheetRows = 4;
constexpr std::uint32_t kSparkleFrameCount = 16;

struct UvRect
{
    float minU = 0.0f;
    float minV = 0.0f;
    float maxU = 1.0f;
    float maxV = 1.0f;
};

[[nodiscard]] UvRect fruitUv(FruitSymbol symbol);
[[nodiscard]] UvRect sparkleUv(std::uint32_t frame);
[[nodiscard]] int fruitPayout(FruitSymbol symbol);

}
