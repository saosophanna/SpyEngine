#include "ecs/systems/system_scheduler.hpp"

#include "ecs/registry/registry.hpp"

namespace spy::ecs {

SystemScheduler::SystemScheduler(std::pmr::memory_resource* resource)
    : m_systems(resource)
{
}

bool SystemScheduler::addSystem(std::unique_ptr<ISystem> system, SystemPhase phase)
{
    if (!system)
    {
        return false;
    }

    m_systems.push_back(Entry{std::move(system), phase});

    return true;
}

bool SystemScheduler::update(Registry& registry, float32 deltaTime)
{
    bool success = true;

    for (usize phaseIndex = 0; phaseIndex < kSystemPhaseCount; ++phaseIndex)
    {
        const auto phase = static_cast<SystemPhase>(phaseIndex);

        if (!updatePhase(registry, phase, deltaTime))
        {
            success = false;
        }
    }

    return success;
}

bool SystemScheduler::updatePhase(Registry& registry, SystemPhase phase, float32 deltaTime)
{
    bool success = true;

    for (Entry& entry : m_systems)
    {
        if (entry.phase != phase)
        {
            continue;
        }

        if (!entry.system->update(registry, deltaTime))
        {
            success = false;
        }
    }

    return success;
}

usize SystemScheduler::systemCount() const noexcept
{
    return m_systems.size();
}

void SystemScheduler::clear() noexcept
{
    m_systems.clear();
}

}
