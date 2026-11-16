#pragma once

#include <memory_resource>
#include <span>
#include <utility>

#include "core/utilities/types/types.hpp"
#include "ecs/component/component.hpp"
#include "ecs/entity/entity_id.hpp"
#include "ecs/storage/sparse_set.hpp"
#include "ecs/storage/storage_base.hpp"

namespace spy::ecs {

template<Component T>
class ComponentStorage final : public IComponentStorage
{
public:
    explicit ComponentStorage(
        std::pmr::memory_resource* resource = std::pmr::get_default_resource())
        : m_components(resource)
    {
    }

    template<typename... Args>
    T* emplace(Entity entity, Args&&... args)
    {
        return m_components.emplace(entity, std::forward<Args>(args)...);
    }

    [[nodiscard]] T* tryGet(Entity entity) noexcept
    {
        return m_components.tryGet(entity);
    }

    [[nodiscard]] const T* tryGet(Entity entity) const noexcept
    {
        return m_components.tryGet(entity);
    }

    [[nodiscard]] T& get(Entity entity) noexcept
    {
        return m_components.get(entity);
    }

    [[nodiscard]] const T& get(Entity entity) const noexcept
    {
        return m_components.get(entity);
    }

    void reserve(usize capacity)
    {
        m_components.reserve(capacity);
    }

    [[nodiscard]] usize size() const noexcept override
    {
        return m_components.size();
    }

    [[nodiscard]] bool contains(Entity entity) const noexcept override
    {
        return m_components.contains(entity);
    }

    [[nodiscard]] std::span<const Entity> entities() const noexcept override
    {
        return m_components.entities();
    }

    bool remove(Entity entity) override
    {
        return m_components.remove(entity);
    }

    void clear() noexcept override
    {
        m_components.clear();
    }

private:
    SparseSet<T> m_components;
};

}
