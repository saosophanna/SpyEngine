#pragma once

#include <memory_resource>
#include <span>
#include <utility>
#include <vector>

#include "core/utilities/types/types.hpp"
#include "ecs/component/component.hpp"
#include "ecs/entity/entity_id.hpp"

namespace spy::ecs {

template<Component T>
class SparseSet
{
public:
    explicit SparseSet(
        std::pmr::memory_resource* resource = std::pmr::get_default_resource())
        : m_sparse(resource)
        , m_entities(resource)
        , m_components(resource)
    {
    }

    template<typename... Args>
    T* emplace(Entity entity, Args&&... args)
    {
        if (!entity.isValid())
        {
            return nullptr;
        }

        if (static_cast<usize>(entity.index()) >= m_sparse.size())
        {
            m_sparse.resize(static_cast<usize>(entity.index()) + 1, 0);
        }

        if (m_sparse[entity.index()] != 0)
        {
            return nullptr;
        }

        m_entities.push_back(entity);

        try
        {
            m_components.emplace_back(std::forward<Args>(args)...);
        }
        catch (...)
        {
            m_entities.pop_back();
            throw;
        }

        m_sparse[entity.index()] = static_cast<uint32>(m_entities.size());

        return &m_components.back();
    }

    bool remove(Entity entity)
    {
        if (!contains(entity))
        {
            return false;
        }

        const uint32 denseIndex = m_sparse[entity.index()] - 1;
        const uint32 lastIndex = static_cast<uint32>(m_entities.size() - 1);

        if (denseIndex != lastIndex)
        {
            const Entity moved = m_entities[lastIndex];

            m_entities[denseIndex] = moved;
            m_components[denseIndex] = std::move(m_components[lastIndex]);
            m_sparse[moved.index()] = denseIndex + 1;
        }

        m_entities.pop_back();
        m_components.pop_back();
        m_sparse[entity.index()] = 0;

        return true;
    }

    [[nodiscard]] bool contains(Entity entity) const noexcept
    {
        if (!entity.isValid() || static_cast<usize>(entity.index()) >= m_sparse.size())
        {
            return false;
        }

        const uint32 position = m_sparse[entity.index()];

        return position != 0 && m_entities[position - 1] == entity;
    }

    [[nodiscard]] T* tryGet(Entity entity) noexcept
    {
        return contains(entity) ? &m_components[m_sparse[entity.index()] - 1] : nullptr;
    }

    [[nodiscard]] const T* tryGet(Entity entity) const noexcept
    {
        return contains(entity) ? &m_components[m_sparse[entity.index()] - 1] : nullptr;
    }

    [[nodiscard]] T& get(Entity entity) noexcept
    {
        return m_components[m_sparse[entity.index()] - 1];
    }

    [[nodiscard]] const T& get(Entity entity) const noexcept
    {
        return m_components[m_sparse[entity.index()] - 1];
    }

    [[nodiscard]] usize size() const noexcept
    {
        return m_entities.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return m_entities.empty();
    }

    [[nodiscard]] std::span<const Entity> entities() const noexcept
    {
        return std::span<const Entity>(m_entities.data(), m_entities.size());
    }

    void reserve(usize capacity)
    {
        m_entities.reserve(capacity);
        m_components.reserve(capacity);
    }

    void clear() noexcept
    {
        m_sparse.clear();
        m_entities.clear();
        m_components.clear();
    }

private:
    std::pmr::vector<uint32> m_sparse;
    std::pmr::vector<Entity> m_entities;
    std::pmr::vector<T> m_components;
};

}
