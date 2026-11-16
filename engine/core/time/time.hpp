#pragma once

#include "core/time/clock.hpp"
#include "core/time/fixed_timestep.hpp"
#include "core/time/frame_time.hpp"
#include "core/time/timestep.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::time {

class Time
{
public:
    Time();
    explicit Time(float64 fixedDelta);

    void initialize();

    void beginFrame();
    void endFrame();

    void setTimeScale(float64 scale);
    [[nodiscard]] float64 timeScale() const noexcept;

    void setPaused(bool paused);
    [[nodiscard]] bool isPaused() const noexcept;

    void setFixedDeltaTime(float64 fixedDelta);
    [[nodiscard]] float64 fixedDeltaTime() const noexcept;
    [[nodiscard]] float64 interpolationAlpha() const noexcept;

    [[nodiscard]] float64 deltaTime() const noexcept;
    [[nodiscard]] float64 unscaledDeltaTime() const noexcept;
    [[nodiscard]] float64 elapsedTime() const noexcept;
    [[nodiscard]] float64 unscaledElapsedTime() const noexcept;

    [[nodiscard]] const FrameTime& frameTime() const noexcept;

    [[nodiscard]] bool shouldRunFixedUpdate() const noexcept;
    bool consumeFixedUpdate();

private:
    FrameTime m_frameTime;
    FixedTimeStep m_fixedTimeStep;
    Clock::TimePoint m_lastFrame;
    bool m_hasLastFrame = false;
    bool m_paused = false;
    float64 m_timeScale = 1.0;
    float64 m_smoothedDelta = 0.0;
};

}
