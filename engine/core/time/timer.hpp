#pragma once

#include <cstdint>

#include "core/utilities/types/types.hpp"

namespace spy::time {

class Timer
{
public:
    explicit Timer(float64 duration = 0.0, bool repeating = false);

    void start();
    void stop();
    void reset();

    void pause();
    void resume();

    uint32 tick(float64 deltaTime);

    [[nodiscard]] bool isRunning() const noexcept;
    [[nodiscard]] bool isPaused() const noexcept;
    [[nodiscard]] bool isFinished() const noexcept;

    [[nodiscard]] float64 duration() const noexcept;
    [[nodiscard]] float64 elapsed() const noexcept;
    [[nodiscard]] float64 remaining() const noexcept;
    [[nodiscard]] float64 progress() const noexcept;

    void setDuration(float64 duration);
    void setRepeating(bool repeating) noexcept;

private:
    float64 m_duration = 0.0;
    float64 m_elapsed = 0.0;

    bool m_running = false;
    bool m_paused = false;
    bool m_repeating = false;
    bool m_finished = false;
};

}
