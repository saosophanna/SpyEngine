#pragma once

#include "core/utilities/types/types.hpp"

namespace spy::ecs {

enum class SystemPhase : uint8
{
    PreUpdate = 0,
    FixedUpdate,
    Update,
    PostUpdate,
    Render,
    Count
};

inline constexpr usize kSystemPhaseCount = static_cast<usize>(SystemPhase::Count);

}
