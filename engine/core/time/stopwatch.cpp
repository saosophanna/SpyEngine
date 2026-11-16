#include <chrono>
#include <cstdint>

#include "core/time/stopwatch.hpp"

namespace spy::time {

void Stopwatch::start()
{
    m_start = Clock::now();
    m_stop = m_start;
    m_running = true;
}

void Stopwatch::stop()
{
    if (!m_running)
    {
        return;
    }

    m_stop = Clock::now();
    m_running = false;
}

void Stopwatch::reset()
{
    m_start = Clock::TimePoint{};
    m_stop = m_start;
    m_running = false;
}

bool Stopwatch::isRunning() const noexcept
{
    return m_running;
}

double Stopwatch::elapsedSeconds() const
{
    return std::chrono::duration<double>(measurementEnd() - m_start).count();
}

double Stopwatch::elapsedMilliseconds() const
{
    return std::chrono::duration<double, std::milli>(measurementEnd() - m_start).count();
}

std::uint64_t Stopwatch::elapsedNanoseconds() const
{
    const auto elapsed =
        std::chrono::duration_cast<std::chrono::nanoseconds>(measurementEnd() - m_start).count();

    return elapsed > 0 ? static_cast<std::uint64_t>(elapsed) : 0;
}

Clock::TimePoint Stopwatch::measurementEnd() const noexcept
{
    return m_running ? Clock::now() : m_stop;
}

}
