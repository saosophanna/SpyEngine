#pragma once

#include <memory>
#include <memory_resource>
#include <vector>

#include "core/utilities/memory/non_copyable.hpp"
#include "core/utilities/types/types.hpp"
#include "ecs/systems/system.hpp"
#include "ecs/systems/system_phase.hpp"

namespace spy::ecs {

class Registry;

class SystemScheduler final : public NonCopyable
{
public:
    explicit SystemScheduler(
        std::pmr::memory_resource* resource = std::pmr::get_default_resource());

    [[nodiscard]] bool addSystem(
        std::unique_ptr<ISystem> system, SystemPhase phase = SystemPhase::Update);

    [[nodiscard]] bool update(Registry& registry, float32 deltaTime);
    [[nodiscard]] bool updatePhase(
        Registry& registry, SystemPhase phase, float32 deltaTime);

    [[nodiscard]] usize systemCount() const noexcept;

    void clear() noexcept;

private:
    struct Entry
    {
        std::unique_ptr<ISystem> system;
        SystemPhase phase = SystemPhase::Update;
    };

    std::pmr::vector<Entry> m_systems;
};

}
