#include <chrono>
#include <cstdint>

#include "core/time/clock.hpp"

namespace spy::time {

Clock::Clock()
    : m_start(ClockType::now())
{
}

void Clock::reset()
{
    m_start = ClockType::now();
}

double Clock::elapsedSeconds() const
{
    return std::chrono::duration<double>(ClockType::now() - m_start).count();
}

double Clock::elapsedMilliseconds() const
{
    return std::chrono::duration<double, std::milli>(ClockType::now() - m_start).count();
}

std::uint64_t Clock::elapsedNanoseconds() const
{
    const auto elapsed =
        std::chrono::duration_cast<std::chrono::nanoseconds>(ClockType::now() - m_start).count();

    return elapsed > 0 ? static_cast<std::uint64_t>(elapsed) : 0;
}

Clock::TimePoint Clock::now() noexcept
{
    return ClockType::now();
}

double Clock::nowSeconds()
{
    return std::chrono::duration<double>(ClockType::now().time_since_epoch()).count();
}

std::uint64_t Clock::nowNanoseconds()
{
    const auto elapsed =
        std::chrono::duration_cast<std::chrono::nanoseconds>(ClockType::now().time_since_epoch())
            .count();

    return elapsed > 0 ? static_cast<std::uint64_t>(elapsed) : 0;
}

}
