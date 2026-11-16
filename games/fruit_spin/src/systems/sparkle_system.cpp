#include "systems/sparkle_system.hpp"

#include "components/sparkle.hpp"
#include "core/utilities/containers/small_vector.hpp"

namespace fruit_spin
{

void updateSparkles(spy::ecs::Registry& registry, float deltaTime)
{
    spy::SmallVector<spy::ecs::Entity, 64> expired;

    registry.each<Sparkle>(
        [&](spy::ecs::Entity entity, Sparkle& sparkle)
        {
            sparkle.age += deltaTime;
            sparkle.position = sparkle.position + sparkle.velocity * deltaTime;
            sparkle.velocity = sparkle.velocity * (1.0f - 1.4f * deltaTime);
            sparkle.rotation += 2.2f * deltaTime;

            if (sparkle.age >= sparkle.lifetime)
            {
                expired.pushBack(entity);
            }
        });

    for (const spy::ecs::Entity entity : expired)
    {
        (void)registry.destroyEntity(entity);
    }
}

spy::ecs::Entity spawnSparkle(spy::ecs::Registry& registry,
                              const spy::math::Vec2& position,
                              const spy::math::Vec2& velocity,
                              float lifetime,
                              float size)
{
    const spy::ecs::Entity entity = registry.createEntity();
    Sparkle sparkle{};
    sparkle.position = position;
    sparkle.velocity = velocity;
    sparkle.lifetime = lifetime;
    sparkle.size = size;
    registry.add<Sparkle>(entity, sparkle);
    return entity;
}

}
