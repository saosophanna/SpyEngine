#pragma once

#include "core/time/clock.hpp"
#include "core/time/fixed_timestep.hpp"
#include "core/time/frame_time.hpp"
#include "core/time/timestep.hpp"

namespace spy::time {

class Time
{
public:
    Time();
    explicit Time(double fixedDelta);

    void initialize();

    void beginFrame();
    void endFrame();

    void setTimeScale(double scale);
    [[nodiscard]] double timeScale() const noexcept;

    void setPaused(bool paused);
    [[nodiscard]] bool isPaused() const noexcept;

    void setFixedDeltaTime(double fixedDelta);
    [[nodiscard]] double fixedDeltaTime() const noexcept;
    [[nodiscard]] double interpolationAlpha() const noexcept;

    [[nodiscard]] double deltaTime() const noexcept;
    [[nodiscard]] double unscaledDeltaTime() const noexcept;
    [[nodiscard]] double elapsedTime() const noexcept;
    [[nodiscard]] double unscaledElapsedTime() const noexcept;

    [[nodiscard]] const FrameTime& frameTime() const noexcept;

    [[nodiscard]] bool shouldRunFixedUpdate() const noexcept;
    bool consumeFixedUpdate();

private:
    FrameTime m_frameTime;
    FixedTimeStep m_fixedTimeStep;
    Clock::TimePoint m_lastFrame;
    bool m_hasLastFrame = false;
    bool m_paused = false;
    double m_timeScale = 1.0;
    double m_smoothedDelta = 0.0;
};

}
