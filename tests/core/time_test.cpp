#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <thread>

#include "core/time/clock.hpp"
#include "core/time/fixed_timestep.hpp"
#include "core/time/frame_limiter.hpp"
#include "core/time/stopwatch.hpp"
#include "core/time/time.hpp"
#include "core/time/timer.hpp"

namespace {

int g_failures = 0;

void check(bool condition, const char* expression, int line)
{
    if (!condition)
    {
        std::fprintf(stderr, "FAILED: %s (line %d)\n", expression, line);
        ++g_failures;
    }
}

bool approx(double first, double second, double tolerance = 1.0e-6)
{
    return std::fabs(first - second) <= tolerance;
}

void sleepMilliseconds(int milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

}

#define SPY_CHECK(expression) check((expression), #expression, __LINE__)

int main()
{
    using namespace spy::time;

    const Timestep frameStep = Timestep::fromMilliseconds(16.0);
    SPY_CHECK(approx(frameStep.seconds, 0.016));
    SPY_CHECK(approx(frameStep.milliseconds(), 16.0));
    SPY_CHECK(approx(Timestep::fromSeconds(1.0).clampedTo(kMaxFrameDelta).seconds, kMaxFrameDelta));
    SPY_CHECK(approx(Timestep::fromSeconds(-1.0).clampedTo(kMaxFrameDelta).seconds, 0.0));
    SPY_CHECK(approx(Timestep::fromSeconds(std::nan("")).clampedTo(kMaxFrameDelta).seconds, 0.0));
    SPY_CHECK(Timestep{}.isZero());
    SPY_CHECK(!frameStep.isZero());

    Clock clock;
    const double initialElapsed = clock.elapsedSeconds();
    sleepMilliseconds(2);
    const double laterElapsed = clock.elapsedSeconds();
    SPY_CHECK(laterElapsed >= initialElapsed);
    SPY_CHECK(laterElapsed >= 0.001);
    SPY_CHECK(clock.elapsedMilliseconds() >= 0.0);
    SPY_CHECK(clock.elapsedNanoseconds() > 0);
    SPY_CHECK(Clock::nowSeconds() > 0.0);
    SPY_CHECK(Clock::nowNanoseconds() > 0);
    clock.reset();
    SPY_CHECK(clock.elapsedSeconds() < laterElapsed);

    FixedTimeStep sixtyHertz(1.0 / 60.0);
    sixtyHertz.addFrameTime(sixtyHertz.deltaTime() * 3.5);
    std::uint32_t steps = 0;
    while (sixtyHertz.canConsumeStep())
    {
        sixtyHertz.consumeStep();
        ++steps;
    }
    SPY_CHECK(steps == 3);
    SPY_CHECK(sixtyHertz.stepsThisFrame() == 3);
    SPY_CHECK(sixtyHertz.simulationTick() == 3);
    SPY_CHECK(approx(sixtyHertz.interpolationAlpha(), 0.5));

    FixedTimeStep oneTwentyHertz(1.0 / 120.0);
    oneTwentyHertz.setMaxStepsPerFrame(10);
    oneTwentyHertz.addFrameTime(oneTwentyHertz.deltaTime() * 6.5);
    steps = 0;
    while (oneTwentyHertz.canConsumeStep())
    {
        oneTwentyHertz.consumeStep();
        ++steps;
    }
    SPY_CHECK(steps == 6);
    SPY_CHECK(approx(oneTwentyHertz.interpolationAlpha(), 0.5));

    FixedTimeStep catchUp(kDefaultFixedDelta);
    catchUp.setMaxStepsPerFrame(5);
    catchUp.addFrameTime(2.0);
    steps = 0;
    while (catchUp.canConsumeStep())
    {
        catchUp.consumeStep();
        ++steps;
    }
    SPY_CHECK(steps == 5);
    SPY_CHECK(catchUp.reachedStepLimit());
    SPY_CHECK(catchUp.hasPendingStep());
    SPY_CHECK(!catchUp.consumeStep());
    SPY_CHECK(!catchUp.hasPendingStep());
    SPY_CHECK(catchUp.droppedTime() > 0.0);
    SPY_CHECK(catchUp.interpolationAlpha() >= 0.0);
    SPY_CHECK(catchUp.interpolationAlpha() <= 1.0);

    catchUp.reset();
    SPY_CHECK(catchUp.simulationTick() == 0);
    SPY_CHECK(approx(catchUp.droppedTime(), 0.0));
    SPY_CHECK(approx(catchUp.interpolationAlpha(), 0.0));

    FixedTimeStep invalidFixed(0.0);
    SPY_CHECK(approx(invalidFixed.deltaTime(), kDefaultFixedDelta));
    invalidFixed.configure(-1.0);
    SPY_CHECK(approx(invalidFixed.deltaTime(), kDefaultFixedDelta));
    invalidFixed.setMaxStepsPerFrame(0);
    invalidFixed.addFrameTime(1.0);
    SPY_CHECK(invalidFixed.consumeStep());
    SPY_CHECK(!invalidFixed.consumeStep());

    FixedTimeStep nanFixed(kDefaultFixedDelta);
    nanFixed.addFrameTime(std::nan(""));
    SPY_CHECK(!nanFixed.hasPendingStep());
    SPY_CHECK(approx(nanFixed.interpolationAlpha(), 0.0));

    Timer nanDuration(std::nan(""));
    SPY_CHECK(approx(nanDuration.duration(), 0.0));
    nanDuration.start();
    SPY_CHECK(nanDuration.tick(0.016) == 1);

    Timer nanDelta(1.0);
    nanDelta.start();
    SPY_CHECK(nanDelta.tick(std::nan("")) == 0);
    SPY_CHECK(approx(nanDelta.elapsed(), 0.0));

    Timer oneShot(1.0);
    SPY_CHECK(!oneShot.isRunning());
    SPY_CHECK(oneShot.tick(0.5) == 0);
    oneShot.start();
    SPY_CHECK(oneShot.isRunning());
    SPY_CHECK(oneShot.tick(0.4) == 0);
    SPY_CHECK(approx(oneShot.elapsed(), 0.4));
    SPY_CHECK(approx(oneShot.remaining(), 0.6));
    SPY_CHECK(approx(oneShot.progress(), 0.4));
    SPY_CHECK(oneShot.tick(0.7) == 1);
    SPY_CHECK(oneShot.isFinished());
    SPY_CHECK(!oneShot.isRunning());
    SPY_CHECK(approx(oneShot.elapsed(), 1.0));
    SPY_CHECK(approx(oneShot.progress(), 1.0));
    SPY_CHECK(approx(oneShot.remaining(), 0.0));

    Timer pausedTimer(2.0);
    pausedTimer.start();
    pausedTimer.tick(0.5);
    pausedTimer.pause();
    SPY_CHECK(pausedTimer.isPaused());
    SPY_CHECK(pausedTimer.tick(1.0) == 0);
    SPY_CHECK(approx(pausedTimer.elapsed(), 0.5));
    pausedTimer.resume();
    SPY_CHECK(!pausedTimer.isPaused());
    SPY_CHECK(pausedTimer.tick(1.5) == 1);
    SPY_CHECK(pausedTimer.isFinished());

    Timer repeatingTimer(1.0, true);
    repeatingTimer.start();
    SPY_CHECK(repeatingTimer.tick(0.5) == 0);
    SPY_CHECK(repeatingTimer.tick(2.5) == 3);
    SPY_CHECK(repeatingTimer.isRunning());
    SPY_CHECK(!repeatingTimer.isFinished());
    SPY_CHECK(approx(repeatingTimer.elapsed(), 0.0));
    SPY_CHECK(approx(repeatingTimer.progress(), 0.0));

    Timer immediateTimer(0.0);
    immediateTimer.start();
    SPY_CHECK(immediateTimer.tick(0.016) == 1);
    SPY_CHECK(immediateTimer.isFinished());
    SPY_CHECK(!immediateTimer.isRunning());

    Timer repeatingImmediate(0.0, true);
    repeatingImmediate.start();
    SPY_CHECK(repeatingImmediate.tick(0.016) == 1);
    SPY_CHECK(repeatingImmediate.isRunning());
    SPY_CHECK(!repeatingImmediate.isFinished());

    Timer stoppedTimer(1.0);
    stoppedTimer.start();
    stoppedTimer.tick(0.4);
    stoppedTimer.stop();
    SPY_CHECK(!stoppedTimer.isRunning());
    SPY_CHECK(stoppedTimer.tick(1.0) == 0);
    SPY_CHECK(approx(stoppedTimer.elapsed(), 0.4));
    stoppedTimer.reset();
    SPY_CHECK(approx(stoppedTimer.elapsed(), 0.0));
    SPY_CHECK(!stoppedTimer.isFinished());

    Timer configuredTimer;
    configuredTimer.setDuration(0.5);
    configuredTimer.setRepeating(true);
    configuredTimer.start();
    SPY_CHECK(configuredTimer.tick(0.75) == 1);
    SPY_CHECK(approx(configuredTimer.elapsed(), 0.25));

    Stopwatch stopwatch;
    SPY_CHECK(!stopwatch.isRunning());
    SPY_CHECK(approx(stopwatch.elapsedSeconds(), 0.0));
    stopwatch.start();
    SPY_CHECK(stopwatch.isRunning());
    sleepMilliseconds(2);
    stopwatch.stop();
    SPY_CHECK(!stopwatch.isRunning());
    const double stoppedSeconds = stopwatch.elapsedSeconds();
    SPY_CHECK(stoppedSeconds >= 0.001);
    SPY_CHECK(stopwatch.elapsedMilliseconds() >= 1.0);
    SPY_CHECK(stopwatch.elapsedNanoseconds() > 0);
    sleepMilliseconds(2);
    SPY_CHECK(stopwatch.elapsedSeconds() == stoppedSeconds);
    stopwatch.start();
    SPY_CHECK(stopwatch.isRunning());
    stopwatch.reset();
    SPY_CHECK(!stopwatch.isRunning());
    SPY_CHECK(approx(stopwatch.elapsedSeconds(), 0.0));

    Time time;
    time.initialize();
    SPY_CHECK(approx(time.deltaTime(), 0.0));
    SPY_CHECK(approx(time.unscaledDeltaTime(), 0.0));
    SPY_CHECK(time.frameTime().frameCount == 0);
    SPY_CHECK(!time.isPaused());
    SPY_CHECK(approx(time.timeScale(), 1.0));

    time.beginFrame();
    SPY_CHECK(approx(time.deltaTime(), 0.0));
    SPY_CHECK(approx(time.elapsedTime(), 0.0));
    SPY_CHECK(time.frameTime().frameCount == 1);
    time.endFrame();
    SPY_CHECK(approx(time.frameTime().fps, 0.0));

    sleepMilliseconds(2);
    time.beginFrame();
    SPY_CHECK(time.unscaledDeltaTime() > 0.0);
    SPY_CHECK(time.unscaledDeltaTime() <= kMaxFrameDelta);
    SPY_CHECK(approx(time.deltaTime(), time.unscaledDeltaTime()));
    SPY_CHECK(time.elapsedTime() > 0.0);
    time.endFrame();
    SPY_CHECK(time.frameTime().fps > 0.0);
    SPY_CHECK(time.frameTime().smoothedFps > 0.0);

    time.setTimeScale(0.5);
    sleepMilliseconds(2);
    time.beginFrame();
    SPY_CHECK(approx(time.deltaTime(), time.unscaledDeltaTime() * 0.5));
    SPY_CHECK(approx(time.frameTime().timeScale, 0.5));
    time.endFrame();

    time.setTimeScale(0.0);
    const double frozenElapsed = time.elapsedTime();
    sleepMilliseconds(2);
    time.beginFrame();
    SPY_CHECK(approx(time.deltaTime(), 0.0));
    SPY_CHECK(approx(time.elapsedTime(), frozenElapsed));
    SPY_CHECK(time.unscaledDeltaTime() > 0.0);
    time.endFrame();

    time.setTimeScale(1.0);
    time.setPaused(true);
    const double pausedElapsed = time.elapsedTime();
    sleepMilliseconds(2);
    time.beginFrame();
    SPY_CHECK(approx(time.deltaTime(), 0.0));
    SPY_CHECK(approx(time.elapsedTime(), pausedElapsed));
    SPY_CHECK(!time.shouldRunFixedUpdate());
    time.endFrame();
    time.setPaused(false);

    time.setFixedDeltaTime(0.02);
    SPY_CHECK(approx(time.fixedDeltaTime(), 0.02));
    time.initialize();
    SPY_CHECK(approx(time.fixedDeltaTime(), 0.02));
    SPY_CHECK(approx(time.unscaledElapsedTime(), 0.0));
    SPY_CHECK(time.frameTime().frameCount == 0);

    FrameLimiter limiter;
    SPY_CHECK(limiter.isUnlimited());
    SPY_CHECK(limiter.targetFps() == 0);
    limiter.setTargetFps(60);
    SPY_CHECK(limiter.targetFps() == 60);
    SPY_CHECK(!limiter.isUnlimited());
    limiter.setTargetFps(1000);
    limiter.beginFrame();
    limiter.endFrame();
    limiter.setUnlimited();
    SPY_CHECK(limiter.isUnlimited());
    limiter.beginFrame();
    limiter.endFrame();

    if (g_failures != 0)
    {
        std::fprintf(stderr, "%d checks failed\n", g_failures);
        return 1;
    }

    return 0;
}
