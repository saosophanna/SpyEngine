#pragma once

#include <cstdint>

namespace spy::time {

struct FrameTime
{
    double deltaTime = 0.0;
    double unscaledDeltaTime = 0.0;

    double elapsedTime = 0.0;
    double unscaledElapsedTime = 0.0;

    double timeScale = 1.0;

    double fps = 0.0;
    double smoothedFps = 0.0;

    std::uint64_t frameCount = 0;
    std::uint64_t simulationTick = 0;

    double droppedSimulationTime = 0.0;
};

}
