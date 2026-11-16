#include <chrono>
#include <cstdint>

#include "core/time/clock.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::time {

Clock::Clock()
    : m_start(ClockType::now())
{
}

void Clock::reset()
{
    m_start = ClockType::now();
}

float64 Clock::elapsedSeconds() const
{
    return std::chrono::duration<float64>(ClockType::now() - m_start).count();
}

float64 Clock::elapsedMilliseconds() const
{
    return std::chrono::duration<float64, std::milli>(ClockType::now() - m_start).count();
}

uint64 Clock::elapsedNanoseconds() const
{
    const auto elapsed =
        std::chrono::duration_cast<std::chrono::nanoseconds>(ClockType::now() - m_start).count();

    return elapsed > 0 ? static_cast<uint64>(elapsed) : 0;
}

Clock::TimePoint Clock::now() noexcept
{
    return ClockType::now();
}

float64 Clock::nowSeconds()
{
    return std::chrono::duration<float64>(ClockType::now().time_since_epoch()).count();
}

uint64 Clock::nowNanoseconds()
{
    const auto elapsed =
        std::chrono::duration_cast<std::chrono::nanoseconds>(ClockType::now().time_since_epoch())
            .count();

    return elapsed > 0 ? static_cast<uint64>(elapsed) : 0;
}

}
