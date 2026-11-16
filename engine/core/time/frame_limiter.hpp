#pragma once

#include <chrono>
#include <cstdint>

#include "core/time/clock.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::time {

class FrameLimiter
{
public:
    FrameLimiter() = default;

    void setTargetFps(uint32 fps);
    void setUnlimited();

    [[nodiscard]] uint32 targetFps() const noexcept;
    [[nodiscard]] bool isUnlimited() const noexcept;

    void beginFrame();
    void endFrame();

private:
    [[nodiscard]] Clock::Duration targetFrameDuration() const noexcept;

    uint32 m_targetFps = 0;
    Clock::TimePoint m_frameStart;
};

}
