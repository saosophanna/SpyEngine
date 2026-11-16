#include "gameplay/fruit_symbols.hpp"

namespace fruit_spin
{

namespace
{

UvRect cellUv(std::uint32_t index, std::uint32_t columns, std::uint32_t rows)
{
    const std::uint32_t column = index % columns;
    const std::uint32_t row = index / columns;
    const float width = 1.0f / static_cast<float>(columns);
    const float height = 1.0f / static_cast<float>(rows);

    return UvRect{static_cast<float>(column) * width,
                  static_cast<float>(row) * height,
                  static_cast<float>(column + 1) * width,
                  static_cast<float>(row + 1) * height};
}

}

UvRect fruitUv(FruitSymbol symbol)
{
    return cellUv(static_cast<std::uint32_t>(symbol), kFruitSheetColumns, kFruitSheetRows);
}

UvRect sparkleUv(std::uint32_t frame)
{
    return cellUv(frame % kSparkleFrameCount, kSparkleSheetColumns, kSparkleSheetRows);
}

int fruitPayout(FruitSymbol symbol)
{
    switch (symbol)
    {
        case FruitSymbol::Cherry:
            return 10;
        case FruitSymbol::Strawberry:
            return 15;
        case FruitSymbol::Watermelon:
            return 20;
        case FruitSymbol::Orange:
            return 25;
        case FruitSymbol::Lemon:
            return 30;
        case FruitSymbol::Grape:
            return 40;
        case FruitSymbol::Apple:
            return 50;
        case FruitSymbol::Banana:
            return 60;
        case FruitSymbol::Pineapple:
            return 80;
        case FruitSymbol::Blueberry:
            return 100;
        case FruitSymbol::Count:
        default:
            return 0;
    }
}

}
