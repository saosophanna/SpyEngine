#pragma once

#include <cstdint>

namespace spy::time {

class Timer
{
public:
    explicit Timer(double duration = 0.0, bool repeating = false);

    void start();
    void stop();
    void reset();

    void pause();
    void resume();

    std::uint32_t tick(double deltaTime);

    [[nodiscard]] bool isRunning() const noexcept;
    [[nodiscard]] bool isPaused() const noexcept;
    [[nodiscard]] bool isFinished() const noexcept;

    [[nodiscard]] double duration() const noexcept;
    [[nodiscard]] double elapsed() const noexcept;
    [[nodiscard]] double remaining() const noexcept;
    [[nodiscard]] double progress() const noexcept;

    void setDuration(double duration);
    void setRepeating(bool repeating) noexcept;

private:
    double m_duration = 0.0;
    double m_elapsed = 0.0;

    bool m_running = false;
    bool m_paused = false;
    bool m_repeating = false;
    bool m_finished = false;
};

}
