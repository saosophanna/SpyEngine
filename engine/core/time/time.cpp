#include <chrono>
#include <cmath>

#include "core/time/time.hpp"

namespace spy::time {

namespace {

constexpr double kFpsSmoothing = 0.1;

}

Time::Time()
    : Time(kDefaultFixedDelta)
{
}

Time::Time(double fixedDelta)
    : m_fixedTimeStep(fixedDelta)
{
    initialize();
}

void Time::initialize()
{
    m_frameTime = FrameTime{};
    m_fixedTimeStep.reset();
    m_lastFrame = Clock::TimePoint{};
    m_hasLastFrame = false;
    m_paused = false;
    m_timeScale = 1.0;
    m_smoothedDelta = 0.0;
}

void Time::beginFrame()
{
    const Clock::TimePoint current = Clock::now();
    const double measuredSeconds =
        m_hasLastFrame ? std::chrono::duration<double>(current - m_lastFrame).count() : 0.0;

    m_lastFrame = current;
    m_hasLastFrame = true;

    const Timestep measured = Timestep::fromSeconds(measuredSeconds).clampedTo(kMaxFrameDelta);
    const double effectiveScale = m_paused ? 0.0 : m_timeScale;

    m_frameTime.unscaledDeltaTime = measured.seconds;
    m_frameTime.deltaTime = measured.scaledBy(effectiveScale).seconds;
    m_frameTime.timeScale = effectiveScale;
    m_frameTime.unscaledElapsedTime += m_frameTime.unscaledDeltaTime;
    m_frameTime.elapsedTime += m_frameTime.deltaTime;
    ++m_frameTime.frameCount;

    m_fixedTimeStep.addFrameTime(m_frameTime.deltaTime);
}

void Time::endFrame()
{
    const double measured = m_frameTime.unscaledDeltaTime;

    if (measured > 0.0)
    {
        m_smoothedDelta = m_smoothedDelta > 0.0
                              ? kFpsSmoothing * measured + (1.0 - kFpsSmoothing) * m_smoothedDelta
                              : measured;
        m_frameTime.fps = 1.0 / measured;
        m_frameTime.smoothedFps = 1.0 / m_smoothedDelta;
    }
    else
    {
        m_frameTime.fps = 0.0;
        m_frameTime.smoothedFps = 0.0;
    }

    if (m_fixedTimeStep.reachedStepLimit())
    {
        m_fixedTimeStep.discardExcessBacklog();
    }

    m_frameTime.simulationTick = m_fixedTimeStep.simulationTick();
    m_frameTime.droppedSimulationTime = m_fixedTimeStep.droppedTime();
}

void Time::setTimeScale(double scale)
{
    m_timeScale = std::isfinite(scale) && scale > 0.0 ? scale : 0.0;
}

double Time::timeScale() const noexcept
{
    return m_timeScale;
}

void Time::setPaused(bool paused)
{
    m_paused = paused;
}

bool Time::isPaused() const noexcept
{
    return m_paused;
}

void Time::setFixedDeltaTime(double fixedDelta)
{
    m_fixedTimeStep.configure(fixedDelta);
}

double Time::fixedDeltaTime() const noexcept
{
    return m_fixedTimeStep.deltaTime();
}

double Time::interpolationAlpha() const noexcept
{
    return m_fixedTimeStep.interpolationAlpha();
}

double Time::deltaTime() const noexcept
{
    return m_frameTime.deltaTime;
}

double Time::unscaledDeltaTime() const noexcept
{
    return m_frameTime.unscaledDeltaTime;
}

double Time::elapsedTime() const noexcept
{
    return m_frameTime.elapsedTime;
}

double Time::unscaledElapsedTime() const noexcept
{
    return m_frameTime.unscaledElapsedTime;
}

const FrameTime& Time::frameTime() const noexcept
{
    return m_frameTime;
}

bool Time::shouldRunFixedUpdate() const noexcept
{
    return !m_paused && m_fixedTimeStep.canConsumeStep();
}

bool Time::consumeFixedUpdate()
{
    return m_fixedTimeStep.consumeStep();
}

}
