#pragma once

#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "core/utilities/types/types.hpp"
#include "ecs/component/component.hpp"
#include "ecs/component/component_storage.hpp"
#include "ecs/entity/entity_id.hpp"
#include "ecs/storage/storage_base.hpp"

namespace spy::ecs {

template<QueryComponent... Ts>
class QueryView
{
    static_assert(sizeof...(Ts) > 0, "Query requires at least one component");

public:
    using Pools = std::tuple<ComponentStorage<std::remove_const_t<Ts>>*...>;

    explicit QueryView(Pools pools)
        : m_pools(pools)
        , m_driver(selectDriver(pools))
    {
    }

    [[nodiscard]] usize size() const noexcept
    {
        if (m_driver == nullptr)
        {
            return 0;
        }

        usize count = 0;

        for (const Entity entity : m_driver->entities())
        {
            if (containsAll(entity))
            {
                ++count;
            }
        }

        return count;
    }

    template<typename Func>
    void each(Func&& func) const
    {
        if (m_driver == nullptr)
        {
            return;
        }

        for (const Entity entity : m_driver->entities())
        {
            if (containsAll(entity))
            {
                invokeEach(func, entity, std::index_sequence_for<Ts...>{});
            }
        }
    }

private:
    static IComponentStorage* selectDriver(const Pools& pools)
    {
        IComponentStorage* driver = nullptr;

        std::apply(
            [&driver](auto*... candidatePools)
            {
                ((driver = smaller(driver, candidatePools)), ...);
            },
            pools);

        return driver;
    }

    static IComponentStorage* smaller(
        IComponentStorage* current, IComponentStorage* candidate) noexcept
    {
        if (current == nullptr || candidate->size() < current->size())
        {
            return candidate;
        }

        return current;
    }

    [[nodiscard]] bool containsAll(Entity entity) const noexcept
    {
        return std::apply(
            [entity](auto*... pools)
            {
                return (pools->contains(entity) && ...);
            },
            m_pools);
    }

    template<typename Func, std::size_t... Indices>
    void invokeEach(Func& func, Entity entity, std::index_sequence<Indices...>) const
    {
        std::invoke(
            func,
            entity,
            static_cast<std::tuple_element_t<Indices, std::tuple<Ts...>>&>(
                std::get<Indices>(m_pools)->get(entity))...);
    }

    Pools m_pools;
    IComponentStorage* m_driver = nullptr;
};

}
