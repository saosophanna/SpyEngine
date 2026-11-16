#pragma once

#include <cstdint>

#include "core/time/timestep.hpp"

namespace spy::time {

class FixedTimeStep
{
public:
    explicit FixedTimeStep(double fixedDelta = kDefaultFixedDelta);

    void configure(double fixedDelta);

    void addFrameTime(double deltaTime);

    [[nodiscard]] bool hasPendingStep() const noexcept;
    [[nodiscard]] bool canConsumeStep() const noexcept;
    bool consumeStep();

    [[nodiscard]] double deltaTime() const noexcept;
    [[nodiscard]] double interpolationAlpha() const noexcept;

    [[nodiscard]] std::uint32_t stepsThisFrame() const noexcept;
    [[nodiscard]] bool reachedStepLimit() const noexcept;
    [[nodiscard]] std::uint64_t simulationTick() const noexcept;
    [[nodiscard]] double droppedTime() const noexcept;

    void discardExcessBacklog();
    void reset();

    void setMaxStepsPerFrame(std::uint32_t maxSteps);

private:
    double m_fixedDelta;
    double m_accumulator = 0.0;
    double m_droppedTime = 0.0;
    std::uint64_t m_simulationTick = 0;
    std::uint32_t m_maxStepsPerFrame = kMaxFixedStepsPerFrame;
    std::uint32_t m_stepsThisFrame = 0;
};

}
