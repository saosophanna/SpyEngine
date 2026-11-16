#pragma once

#include <cstdint>

#include "core/time/timestep.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::time {

class FixedTimeStep
{
public:
    explicit FixedTimeStep(float64 fixedDelta = kDefaultFixedDelta);

    void configure(float64 fixedDelta);

    void addFrameTime(float64 deltaTime);

    [[nodiscard]] bool hasPendingStep() const noexcept;
    [[nodiscard]] bool canConsumeStep() const noexcept;
    bool consumeStep();

    [[nodiscard]] float64 deltaTime() const noexcept;
    [[nodiscard]] float64 interpolationAlpha() const noexcept;

    [[nodiscard]] uint32 stepsThisFrame() const noexcept;
    [[nodiscard]] bool reachedStepLimit() const noexcept;
    [[nodiscard]] uint64 simulationTick() const noexcept;
    [[nodiscard]] float64 droppedTime() const noexcept;

    void discardExcessBacklog();
    void reset();

    void setMaxStepsPerFrame(uint32 maxSteps);

private:
    float64 m_fixedDelta;
    float64 m_accumulator = 0.0;
    float64 m_droppedTime = 0.0;
    uint64 m_simulationTick = 0;
    uint32 m_maxStepsPerFrame = kMaxFixedStepsPerFrame;
    uint32 m_stepsThisFrame = 0;
};

}
