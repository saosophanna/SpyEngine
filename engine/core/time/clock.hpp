#pragma once

#include <chrono>
#include <cstdint>

#include "core/utilities/types/types.hpp"

namespace spy::time {

class Clock
{
public:
    using ClockType = std::chrono::steady_clock;
    using TimePoint = ClockType::time_point;
    using Duration = ClockType::duration;

    Clock();

    void reset();

    [[nodiscard]] float64 elapsedSeconds() const;
    [[nodiscard]] float64 elapsedMilliseconds() const;
    [[nodiscard]] uint64 elapsedNanoseconds() const;

    [[nodiscard]] static TimePoint now() noexcept;
    [[nodiscard]] static float64 nowSeconds();
    [[nodiscard]] static uint64 nowNanoseconds();

private:
    TimePoint m_start;
};

}
