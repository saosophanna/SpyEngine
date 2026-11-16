#pragma once

#include "core/utilities/types/handle.hpp"

namespace spy::ecs {

struct EntityTag
{
};

using Entity = Handle<EntityTag>;

inline constexpr Entity kInvalidEntity{};

}
