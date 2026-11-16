#pragma once

#include <cstddef>
#include <vector>

#include "core/math/common/constants.hpp"
#include "core/math/common/math_utils.hpp"
#include "core/math/geometry/rect.hpp"
#include "core/math/vector/vec2.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::math {

struct Polygon
{
    std::vector<Vec2> vertices;

    [[nodiscard]] float32 signedArea() const
    {
        if (vertices.size() < 3)
        {
            return 0.0f;
        }

        float32 doubledArea = 0.0f;

        for (usize index = 0; index < vertices.size(); ++index)
        {
            const Vec2& current = vertices[index];
            const Vec2& next = vertices[(index + 1) % vertices.size()];
            doubledArea += current.cross(next);
        }

        return doubledArea * 0.5f;
    }

    [[nodiscard]] float32 area() const
    {
        return absValue(signedArea());
    }

    [[nodiscard]] Vec2 centroid() const
    {
        if (vertices.empty())
        {
            return Vec2::zero();
        }

        if (vertices.size() < 3)
        {
            Vec2 sum = Vec2::zero();

            for (const Vec2& vertex : vertices)
            {
                sum += vertex;
            }

            return sum / static_cast<float32>(vertices.size());
        }

        const float32 areaValue = signedArea();
        Vec2 accumulated = Vec2::zero();

        for (usize index = 0; index < vertices.size(); ++index)
        {
            const Vec2& current = vertices[index];
            const Vec2& next = vertices[(index + 1) % vertices.size()];
            const float32 cross = current.cross(next);
            accumulated += (current + next) * cross;
        }

        if (absValue(areaValue) <= constants::kEpsilon)
        {
            Vec2 sum = Vec2::zero();

            for (const Vec2& vertex : vertices)
            {
                sum += vertex;
            }

            return sum / static_cast<float32>(vertices.size());
        }

        return accumulated / (6.0f * areaValue);
    }

    [[nodiscard]] float32 perimeter() const
    {
        if (vertices.size() < 2)
        {
            return 0.0f;
        }

        float32 total = 0.0f;

        for (usize index = 0; index < vertices.size(); ++index)
        {
            const Vec2& current = vertices[index];
            const Vec2& next = vertices[(index + 1) % vertices.size()];
            total += current.distance(next);
        }

        return total;
    }

    [[nodiscard]] Rect bounds() const
    {
        if (vertices.empty())
        {
            return Rect{};
        }

        Vec2 low = vertices.front();
        Vec2 high = low;

        for (const Vec2& vertex : vertices)
        {
            low = Vec2{minValue(low.x, vertex.x), minValue(low.y, vertex.y)};
            high = Vec2{maxValue(high.x, vertex.x), maxValue(high.y, vertex.y)};
        }

        return Rect{low, high - low};
    }

    [[nodiscard]] bool contains(const Vec2& point) const
    {
        if (vertices.size() < 3)
        {
            return false;
        }

        bool inside = false;

        for (usize index = 0, previous = vertices.size() - 1; index < vertices.size();
             previous = index++)
        {
            const Vec2& current = vertices[index];
            const Vec2& prior = vertices[previous];
            const Vec2 edge = current - prior;
            const Vec2 toPoint = point - prior;

            if (absValue(edge.cross(toPoint)) <= constants::kEpsilon &&
                toPoint.dot(edge) >= 0.0f && toPoint.dot(edge) <= edge.lengthSquared())
            {
                return true;
            }

            const bool crossesVerticalRange = (current.y > point.y) != (prior.y > point.y);

            if (crossesVerticalRange)
            {
                const float32 intersectionX =
                    (prior.x - current.x) * (point.y - current.y) / (prior.y - current.y) +
                    current.x;

                if (point.x < intersectionX)
                {
                    inside = !inside;
                }
            }
        }

        return inside;
    }
};

}
