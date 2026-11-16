#pragma once

#include <cstdint>

#include "core/utilities/types/types.hpp"

namespace spy::time {

struct FrameTime
{
    float64 deltaTime = 0.0;
    float64 unscaledDeltaTime = 0.0;

    float64 elapsedTime = 0.0;
    float64 unscaledElapsedTime = 0.0;

    float64 timeScale = 1.0;

    float64 fps = 0.0;
    float64 smoothedFps = 0.0;

    uint64 frameCount = 0;
    uint64 simulationTick = 0;

    float64 droppedSimulationTime = 0.0;
};

}
