#include "ecs/entity/entity_manager.hpp"

namespace spy::ecs {

EntityManager::EntityManager(std::pmr::memory_resource* resource)
    : m_slots(resource)
    , m_free(resource)
{
}

Entity EntityManager::create()
{
    if (!m_free.empty())
    {
        const uint32 index = m_free.back();
        m_free.pop_back();

        Slot& slot = m_slots[index];
        slot.alive = true;
        ++m_aliveCount;

        return Entity(index, slot.generation);
    }

    const uint32 index = static_cast<uint32>(m_slots.size());
    m_slots.push_back(Slot{Entity::kInitialGeneration, true});
    ++m_aliveCount;

    return Entity(index, Entity::kInitialGeneration);
}

bool EntityManager::destroy(Entity entity)
{
    if (!isAlive(entity))
    {
        return false;
    }

    Slot& slot = m_slots[entity.index()];
    slot.alive = false;
    ++slot.generation;

    if (slot.generation == 0)
    {
        slot.generation = Entity::kInitialGeneration;
    }

    m_free.push_back(entity.index());
    --m_aliveCount;

    return true;
}

bool EntityManager::isAlive(Entity entity) const noexcept
{
    if (!entity.isValid() || entity.index() >= m_slots.size())
    {
        return false;
    }

    const Slot& slot = m_slots[entity.index()];

    return slot.alive && slot.generation == entity.generation();
}

usize EntityManager::aliveCount() const noexcept
{
    return m_aliveCount;
}

usize EntityManager::capacity() const noexcept
{
    return m_slots.size();
}

void EntityManager::clear()
{
    m_free.clear();
    m_free.reserve(m_slots.size());

    for (usize index = 0; index < m_slots.size(); ++index)
    {
        Slot& slot = m_slots[index];
        slot.alive = false;
        ++slot.generation;

        if (slot.generation == 0)
        {
            slot.generation = Entity::kInitialGeneration;
        }

        m_free.push_back(static_cast<uint32>(index));
    }

    m_aliveCount = 0;
}

}
