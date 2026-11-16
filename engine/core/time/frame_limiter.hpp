#pragma once

#include <chrono>
#include <cstdint>

#include "core/time/clock.hpp"

namespace spy::time {

class FrameLimiter
{
public:
    FrameLimiter() = default;

    void setTargetFps(std::uint32_t fps);
    void setUnlimited();

    [[nodiscard]] std::uint32_t targetFps() const noexcept;
    [[nodiscard]] bool isUnlimited() const noexcept;

    void beginFrame();
    void endFrame();

private:
    [[nodiscard]] Clock::Duration targetFrameDuration() const noexcept;

    std::uint32_t m_targetFps = 0;
    Clock::TimePoint m_frameStart;
};

}
