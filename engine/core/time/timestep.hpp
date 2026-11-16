#pragma once

#include <cstdint>

#include "core/utilities/types/types.hpp"

namespace spy::time {

inline constexpr float64 kDefaultFixedDelta = 1.0 / 60.0;
inline constexpr float64 kMaxFrameDelta = 0.25;
inline constexpr uint32 kMaxFixedStepsPerFrame = 5;

struct Timestep
{
    float64 seconds = 0.0;

    [[nodiscard]] static constexpr Timestep fromSeconds(float64 value) noexcept
    {
        return Timestep{value};
    }

    [[nodiscard]] static constexpr Timestep fromMilliseconds(float64 value) noexcept
    {
        return Timestep{value / 1000.0};
    }

    [[nodiscard]] constexpr float64 milliseconds() const noexcept
    {
        return seconds * 1000.0;
    }

    [[nodiscard]] constexpr Timestep scaledBy(float64 factor) const noexcept
    {
        return Timestep{seconds * factor};
    }

    [[nodiscard]] constexpr Timestep clampedTo(float64 maximumSeconds) const noexcept
    {
        if (!(seconds > 0.0) || !(maximumSeconds > 0.0))
        {
            return Timestep{};
        }

        return seconds < maximumSeconds ? Timestep{seconds} : Timestep{maximumSeconds};
    }

    [[nodiscard]] constexpr bool isZero() const noexcept
    {
        return !(seconds > 0.0);
    }
};

}
