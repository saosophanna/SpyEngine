#pragma once

#include <chrono>
#include <cstdint>

namespace spy::time {

class Clock
{
public:
    using ClockType = std::chrono::steady_clock;
    using TimePoint = ClockType::time_point;
    using Duration = ClockType::duration;

    Clock();

    void reset();

    [[nodiscard]] double elapsedSeconds() const;
    [[nodiscard]] double elapsedMilliseconds() const;
    [[nodiscard]] std::uint64_t elapsedNanoseconds() const;

    [[nodiscard]] static TimePoint now() noexcept;
    [[nodiscard]] static double nowSeconds();
    [[nodiscard]] static std::uint64_t nowNanoseconds();

private:
    TimePoint m_start;
};

}
