#include <chrono>
#include <cstdint>
#include <thread>

#include "core/time/frame_limiter.hpp"

namespace spy::time {

namespace {

constexpr std::chrono::milliseconds kSpinThreshold{1};

}

void FrameLimiter::setTargetFps(std::uint32_t fps)
{
    m_targetFps = fps;
}

void FrameLimiter::setUnlimited()
{
    m_targetFps = 0;
}

std::uint32_t FrameLimiter::targetFps() const noexcept
{
    return m_targetFps;
}

bool FrameLimiter::isUnlimited() const noexcept
{
    return m_targetFps == 0;
}

void FrameLimiter::beginFrame()
{
    m_frameStart = Clock::now();
}

void FrameLimiter::endFrame()
{
    if (m_targetFps == 0)
    {
        return;
    }

    const Clock::TimePoint deadline = m_frameStart + targetFrameDuration();
    const Clock::TimePoint current = Clock::now();

    if (current >= deadline)
    {
        return;
    }

    const Clock::Duration remaining = deadline - current;

    if (remaining > kSpinThreshold)
    {
        std::this_thread::sleep_for(remaining - kSpinThreshold);
    }

    while (Clock::now() < deadline)
    {
        std::this_thread::yield();
    }
}

Clock::Duration FrameLimiter::targetFrameDuration() const noexcept
{
    const double seconds = 1.0 / static_cast<double>(m_targetFps);
    return std::chrono::duration_cast<Clock::Duration>(std::chrono::duration<double>(seconds));
}

}
