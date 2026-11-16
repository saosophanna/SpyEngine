#pragma once

#include <cstdint>

#include "core/time/clock.hpp"

namespace spy::time {

class Stopwatch
{
public:
    Stopwatch() = default;

    void start();
    void stop();
    void reset();

    [[nodiscard]] bool isRunning() const noexcept;

    [[nodiscard]] double elapsedSeconds() const;
    [[nodiscard]] double elapsedMilliseconds() const;
    [[nodiscard]] std::uint64_t elapsedNanoseconds() const;

private:
    [[nodiscard]] Clock::TimePoint measurementEnd() const noexcept;

    Clock::TimePoint m_start;
    Clock::TimePoint m_stop;
    bool m_running = false;
};

}
