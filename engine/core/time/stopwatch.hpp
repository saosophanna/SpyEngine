#pragma once

#include <cstdint>

#include "core/time/clock.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::time {

class Stopwatch
{
public:
    Stopwatch() = default;

    void start();
    void stop();
    void reset();

    [[nodiscard]] bool isRunning() const noexcept;

    [[nodiscard]] float64 elapsedSeconds() const;
    [[nodiscard]] float64 elapsedMilliseconds() const;
    [[nodiscard]] uint64 elapsedNanoseconds() const;

private:
    [[nodiscard]] Clock::TimePoint measurementEnd() const noexcept;

    Clock::TimePoint m_start;
    Clock::TimePoint m_stop;
    bool m_running = false;
};

}
