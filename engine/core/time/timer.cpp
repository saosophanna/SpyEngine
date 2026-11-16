#include <cmath>
#include <cstdint>
#include <limits>

#include "core/time/timer.hpp"

namespace spy::time {

Timer::Timer(double duration, bool repeating)
    : m_duration(std::isfinite(duration) && duration > 0.0 ? duration : 0.0)
    , m_repeating(repeating)
{
}

void Timer::start()
{
    m_elapsed = 0.0;
    m_running = true;
    m_paused = false;
    m_finished = false;
}

void Timer::stop()
{
    m_running = false;
    m_paused = false;
}

void Timer::reset()
{
    m_elapsed = 0.0;
    m_running = false;
    m_paused = false;
    m_finished = false;
}

void Timer::pause()
{
    if (m_running)
    {
        m_paused = true;
    }
}

void Timer::resume()
{
    if (m_running)
    {
        m_paused = false;
    }
}

std::uint32_t Timer::tick(double deltaTime)
{
    if (!m_running || m_paused)
    {
        return 0;
    }

    if (m_duration <= 0.0)
    {
        m_elapsed = 0.0;
        m_running = m_repeating;
        m_paused = false;
        m_finished = !m_repeating;
        return 1;
    }

    if (std::isfinite(deltaTime) && deltaTime > 0.0)
    {
        m_elapsed += deltaTime;
    }

    if (m_elapsed < m_duration)
    {
        return 0;
    }

    if (!m_repeating)
    {
        m_elapsed = m_duration;
        m_running = false;
        m_paused = false;
        m_finished = true;
        return 1;
    }

    const double rawIntervals = std::floor(m_elapsed / m_duration);
    const double intervalLimit = static_cast<double>(std::numeric_limits<std::uint32_t>::max());
    const double intervalCount = rawIntervals < intervalLimit ? rawIntervals : intervalLimit;
    const auto intervals = static_cast<std::uint32_t>(intervalCount);

    m_elapsed -= static_cast<double>(intervals) * m_duration;

    if (m_elapsed >= m_duration)
    {
        m_elapsed = 0.0;
    }

    return intervals;
}

bool Timer::isRunning() const noexcept
{
    return m_running;
}

bool Timer::isPaused() const noexcept
{
    return m_paused;
}

bool Timer::isFinished() const noexcept
{
    return m_finished;
}

double Timer::duration() const noexcept
{
    return m_duration;
}

double Timer::elapsed() const noexcept
{
    return m_elapsed;
}

double Timer::remaining() const noexcept
{
    const double value = m_duration - m_elapsed;
    return value > 0.0 ? value : 0.0;
}

double Timer::progress() const noexcept
{
    if (!(m_duration > 0.0))
    {
        return 1.0;
    }

    const double value = m_elapsed / m_duration;
    return value < 1.0 ? value : 1.0;
}

void Timer::setDuration(double duration)
{
    m_duration = std::isfinite(duration) && duration > 0.0 ? duration : 0.0;
}

void Timer::setRepeating(bool repeating) noexcept
{
    m_repeating = repeating;
}

}
