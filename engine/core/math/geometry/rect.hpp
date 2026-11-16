#pragma once

#include "core/math/common/math_utils.hpp"
#include "core/math/vector/vec2.hpp"

namespace spy::math {

struct Rect
{
    Vec2 position{};
    Vec2 size{};

    [[nodiscard]] constexpr Vec2 center() const
    {
        return position + size * 0.5f;
    }

    [[nodiscard]] constexpr Vec2 min() const
    {
        const Vec2 opposite = position + size;
        return Vec2{minValue(position.x, opposite.x), minValue(position.y, opposite.y)};
    }

    [[nodiscard]] constexpr Vec2 max() const
    {
        const Vec2 opposite = position + size;
        return Vec2{maxValue(position.x, opposite.x), maxValue(position.y, opposite.y)};
    }

    [[nodiscard]] constexpr float width() const
    {
        return absValue(size.x);
    }

    [[nodiscard]] constexpr float height() const
    {
        return absValue(size.y);
    }

    [[nodiscard]] constexpr bool contains(const Vec2& point) const
    {
        const Vec2 low = min();
        const Vec2 high = max();
        return point.x >= low.x && point.x <= high.x && point.y >= low.y && point.y <= high.y;
    }

    [[nodiscard]] constexpr bool overlaps(const Rect& other) const
    {
        const Vec2 low = min();
        const Vec2 high = max();
        const Vec2 otherLow = other.min();
        const Vec2 otherHigh = other.max();
        return low.x <= otherHigh.x && high.x >= otherLow.x && low.y <= otherHigh.y &&
               high.y >= otherLow.y;
    }

    [[nodiscard]] constexpr Rect normalized() const
    {
        const Vec2 low = min();
        return Rect{low, max() - low};
    }

    [[nodiscard]] constexpr Rect expanded(float amount) const
    {
        const Vec2 low = min() - Vec2{amount, amount};
        const Vec2 high = max() + Vec2{amount, amount};
        return Rect{low, high - low};
    }

    [[nodiscard]] constexpr Rect intersection(const Rect& other) const
    {
        const Vec2 low{maxValue(min().x, other.min().x), maxValue(min().y, other.min().y)};
        const Vec2 high{minValue(max().x, other.max().x), minValue(max().y, other.max().y)};

        if (low.x > high.x || low.y > high.y)
        {
            return Rect{low, Vec2::zero()};
        }

        return Rect{low, high - low};
    }
};

}
