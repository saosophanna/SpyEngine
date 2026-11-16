#include <cmath>
#include <cstdint>

#include "core/time/fixed_timestep.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::time {

namespace {

float64 normalizedFixedDelta(float64 value)
{
    return std::isfinite(value) && value > 0.0 ? value : kDefaultFixedDelta;
}

float64 nonNegativeDelta(float64 value)
{
    return std::isfinite(value) && value > 0.0 ? value : 0.0;
}

}

FixedTimeStep::FixedTimeStep(float64 fixedDelta)
    : m_fixedDelta(normalizedFixedDelta(fixedDelta))
{
}

void FixedTimeStep::configure(float64 fixedDelta)
{
    m_fixedDelta = normalizedFixedDelta(fixedDelta);
    m_accumulator = 0.0;
    m_stepsThisFrame = 0;
}

void FixedTimeStep::addFrameTime(float64 deltaTime)
{
    m_stepsThisFrame = 0;
    m_accumulator += nonNegativeDelta(deltaTime);
}

bool FixedTimeStep::hasPendingStep() const noexcept
{
    return m_accumulator >= m_fixedDelta;
}

bool FixedTimeStep::canConsumeStep() const noexcept
{
    return hasPendingStep() && !reachedStepLimit();
}

bool FixedTimeStep::consumeStep()
{
    if (!hasPendingStep())
    {
        return false;
    }

    if (reachedStepLimit())
    {
        discardExcessBacklog();
        return false;
    }

    m_accumulator -= m_fixedDelta;
    ++m_stepsThisFrame;
    ++m_simulationTick;
    return true;
}

float64 FixedTimeStep::deltaTime() const noexcept
{
    return m_fixedDelta;
}

float64 FixedTimeStep::interpolationAlpha() const noexcept
{
    const float64 alpha = m_accumulator / m_fixedDelta;

    if (!(alpha > 0.0))
    {
        return 0.0;
    }

    return alpha < 1.0 ? alpha : 1.0;
}

uint32 FixedTimeStep::stepsThisFrame() const noexcept
{
    return m_stepsThisFrame;
}

bool FixedTimeStep::reachedStepLimit() const noexcept
{
    return m_stepsThisFrame >= m_maxStepsPerFrame;
}

uint64 FixedTimeStep::simulationTick() const noexcept
{
    return m_simulationTick;
}

float64 FixedTimeStep::droppedTime() const noexcept
{
    return m_droppedTime;
}

void FixedTimeStep::discardExcessBacklog()
{
    const float64 remainder = std::fmod(m_accumulator, m_fixedDelta);
    const float64 discarded = m_accumulator - remainder;

    if (discarded <= 0.0)
    {
        return;
    }

    m_accumulator = remainder;
    m_droppedTime += discarded;
}

void FixedTimeStep::reset()
{
    m_accumulator = 0.0;
    m_droppedTime = 0.0;
    m_simulationTick = 0;
    m_stepsThisFrame = 0;
}

void FixedTimeStep::setMaxStepsPerFrame(uint32 maxSteps)
{
    m_maxStepsPerFrame = maxSteps > 0 ? maxSteps : 1;
}

}
