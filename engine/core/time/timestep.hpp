#pragma once

#include <cstdint>

namespace spy::time {

inline constexpr double kDefaultFixedDelta = 1.0 / 60.0;
inline constexpr double kMaxFrameDelta = 0.25;
inline constexpr std::uint32_t kMaxFixedStepsPerFrame = 5;

struct Timestep
{
    double seconds = 0.0;

    [[nodiscard]] static constexpr Timestep fromSeconds(double value) noexcept
    {
        return Timestep{value};
    }

    [[nodiscard]] static constexpr Timestep fromMilliseconds(double value) noexcept
    {
        return Timestep{value / 1000.0};
    }

    [[nodiscard]] constexpr double milliseconds() const noexcept
    {
        return seconds * 1000.0;
    }

    [[nodiscard]] constexpr Timestep scaledBy(double factor) const noexcept
    {
        return Timestep{seconds * factor};
    }

    [[nodiscard]] constexpr Timestep clampedTo(double maximumSeconds) const noexcept
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
