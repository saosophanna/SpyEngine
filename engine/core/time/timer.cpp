#include <cmath>
#include <cstdint>
#include <limits>

#include "core/time/timer.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::time {

Timer::Timer(float64 duration, bool repeating)
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

uint32 Timer::tick(float64 deltaTime)
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

    const float64 rawIntervals = std::floor(m_elapsed / m_duration);
    const float64 intervalLimit = static_cast<float64>(std::numeric_limits<uint32>::max());
    const float64 intervalCount = rawIntervals < intervalLimit ? rawIntervals : intervalLimit;
    const auto intervals = static_cast<uint32>(intervalCount);

    m_elapsed -= static_cast<float64>(intervals) * m_duration;

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

float64 Timer::duration() const noexcept
{
    return m_duration;
}

float64 Timer::elapsed() const noexcept
{
    return m_elapsed;
}

float64 Timer::remaining() const noexcept
{
    const float64 value = m_duration - m_elapsed;
    return value > 0.0 ? value : 0.0;
}

float64 Timer::progress() const noexcept
{
    if (!(m_duration > 0.0))
    {
        return 1.0;
    }

    const float64 value = m_elapsed / m_duration;
    return value < 1.0 ? value : 1.0;
}

void Timer::setDuration(float64 duration)
{
    m_duration = std::isfinite(duration) && duration > 0.0 ? duration : 0.0;
}

void Timer::setRepeating(bool repeating) noexcept
{
    m_repeating = repeating;
}

}
