#include <cmath>
#include <cstdint>

#include "core/time/fixed_timestep.hpp"

namespace spy::time {

namespace {

double normalizedFixedDelta(double value)
{
    return std::isfinite(value) && value > 0.0 ? value : kDefaultFixedDelta;
}

double nonNegativeDelta(double value)
{
    return std::isfinite(value) && value > 0.0 ? value : 0.0;
}

}

FixedTimeStep::FixedTimeStep(double fixedDelta)
    : m_fixedDelta(normalizedFixedDelta(fixedDelta))
{
}

void FixedTimeStep::configure(double fixedDelta)
{
    m_fixedDelta = normalizedFixedDelta(fixedDelta);
    m_accumulator = 0.0;
    m_stepsThisFrame = 0;
}

void FixedTimeStep::addFrameTime(double deltaTime)
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

double FixedTimeStep::deltaTime() const noexcept
{
    return m_fixedDelta;
}

double FixedTimeStep::interpolationAlpha() const noexcept
{
    const double alpha = m_accumulator / m_fixedDelta;

    if (!(alpha > 0.0))
    {
        return 0.0;
    }

    return alpha < 1.0 ? alpha : 1.0;
}

std::uint32_t FixedTimeStep::stepsThisFrame() const noexcept
{
    return m_stepsThisFrame;
}

bool FixedTimeStep::reachedStepLimit() const noexcept
{
    return m_stepsThisFrame >= m_maxStepsPerFrame;
}

std::uint64_t FixedTimeStep::simulationTick() const noexcept
{
    return m_simulationTick;
}

double FixedTimeStep::droppedTime() const noexcept
{
    return m_droppedTime;
}

void FixedTimeStep::discardExcessBacklog()
{
    const double remainder = std::fmod(m_accumulator, m_fixedDelta);
    const double discarded = m_accumulator - remainder;

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

void FixedTimeStep::setMaxStepsPerFrame(std::uint32_t maxSteps)
{
    m_maxStepsPerFrame = maxSteps > 0 ? maxSteps : 1;
}

}
