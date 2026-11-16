#pragma once

#include <memory_resource>
#include <vector>

#include "core/utilities/memory/non_copyable.hpp"
#include "core/utilities/types/types.hpp"
#include "ecs/entity/entity_id.hpp"

namespace spy::ecs {

class EntityManager final : public NonCopyable
{
public:
    explicit EntityManager(
        std::pmr::memory_resource* resource = std::pmr::get_default_resource());

    [[nodiscard]] Entity create();
    [[nodiscard]] bool destroy(Entity entity);
    [[nodiscard]] bool isAlive(Entity entity) const noexcept;
    [[nodiscard]] usize aliveCount() const noexcept;
    [[nodiscard]] usize capacity() const noexcept;

    void clear();

private:
    struct Slot
    {
        Entity::Generation generation = Entity::kInitialGeneration;
        bool alive = false;
    };

    std::pmr::vector<Slot> m_slots;
    std::pmr::vector<uint32> m_free;
    usize m_aliveCount = 0;
};

}
