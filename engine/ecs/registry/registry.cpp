#include "ecs/registry/registry.hpp"

namespace spy::ecs {

Registry::Registry(std::pmr::memory_resource* resource)
    : m_resource(resource != nullptr ? resource : std::pmr::get_default_resource())
    , m_entities(m_resource)
    , m_storages(m_resource)
{
}

Registry::Registry(memory::IAllocator& allocator)
    : m_ownedResource(std::make_unique<memory::AllocatorResource>(allocator))
    , m_resource(m_ownedResource.get())
    , m_entities(m_resource)
    , m_storages(m_resource)
{
}

Entity Registry::createEntity()
{
    return m_entities.create();
}

bool Registry::destroyEntity(Entity entity)
{
    if (!m_entities.isAlive(entity))
    {
        return false;
    }

    for (auto& entry : m_storages)
    {
        entry.second->remove(entity);
    }

    return m_entities.destroy(entity);
}

bool Registry::isAlive(Entity entity) const noexcept
{
    return m_entities.isAlive(entity);
}

usize Registry::entityCount() const noexcept
{
    return m_entities.aliveCount();
}

IComponentStorage* Registry::findStorage(std::type_index type)
{
    const auto it = m_storages.find(type);

    return it != m_storages.end() ? it->second.get() : nullptr;
}

const IComponentStorage* Registry::findStorage(std::type_index type) const
{
    const auto it = m_storages.find(type);

    return it != m_storages.end() ? it->second.get() : nullptr;
}

void Registry::clear()
{
    for (auto& entry : m_storages)
    {
        entry.second->clear();
    }

    m_entities.clear();
}

}
