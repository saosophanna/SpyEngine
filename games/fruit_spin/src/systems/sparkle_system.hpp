#pragma once

#include "core/math/vector/vec2.hpp"
#include "ecs/ecs.hpp"

namespace fruit_spin
{

void updateSparkles(spy::ecs::Registry& registry, float deltaTime);

[[nodiscard]] spy::ecs::Entity spawnSparkle(spy::ecs::Registry& registry,
                                            const spy::math::Vec2& position,
                                            const spy::math::Vec2& velocity,
                                            float lifetime,
                                            float size);

}
