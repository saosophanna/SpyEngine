#pragma once

#include <memory>
#include <memory_resource>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>

#include "core/memory/adapters/allocator_resource.hpp"
#include "core/memory/allocator.hpp"
#include "core/utilities/memory/non_copyable.hpp"
#include "core/utilities/types/types.hpp"
#include "ecs/component/component.hpp"
#include "ecs/component/component_storage.hpp"
#include "ecs/entity/entity_id.hpp"
#include "ecs/entity/entity_manager.hpp"
#include "ecs/query/query_view.hpp"
#include "ecs/storage/storage_base.hpp"

namespace spy::ecs {

class Registry final : public NonCopyable
{
public:
    explicit Registry(
        std::pmr::memory_resource* resource = std::pmr::get_default_resource());
    explicit Registry(memory::IAllocator& allocator);

    [[nodiscard]] Entity createEntity();
    [[nodiscard]] bool destroyEntity(Entity entity);
    [[nodiscard]] bool isAlive(Entity entity) const noexcept;
    [[nodiscard]] usize entityCount() const noexcept;

    template<Component T, typename... Args>
    T* add(Entity entity, Args&&... args);

    template<Component T, typename... Args>
    T* replace(Entity entity, Args&&... args);

    template<Component T>
    bool remove(Entity entity);

    template<Component T>
    [[nodiscard]] T* tryGet(Entity entity);

    template<Component T>
    [[nodiscard]] const T* tryGet(Entity entity) const;

    template<Component T>
    [[nodiscard]] bool has(Entity entity) const;

    template<Component T>
    void reserve(usize capacity);

    template<QueryComponent... Ts>
    [[nodiscard]] QueryView<Ts...> view();

    template<QueryComponent... Ts, typename Func>
    void each(Func&& func);

    void clear();

private:
    using StorageMap =
        std::pmr::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>>;

    template<Component T>
    [[nodiscard]] ComponentStorage<T>& storage();

    [[nodiscard]] IComponentStorage* findStorage(std::type_index type);
    [[nodiscard]] const IComponentStorage* findStorage(std::type_index type) const;

    std::unique_ptr<memory::AllocatorResource> m_ownedResource;
    std::pmr::memory_resource* m_resource = nullptr;
    EntityManager m_entities;
    StorageMap m_storages;
};

template<Component T, typename... Args>
T* Registry::add(Entity entity, Args&&... args)
{
    if (!m_entities.isAlive(entity))
    {
        return nullptr;
    }

    return storage<T>().emplace(entity, std::forward<Args>(args)...);
}

template<Component T, typename... Args>
T* Registry::replace(Entity entity, Args&&... args)
{
    if (!m_entities.isAlive(entity))
    {
        return nullptr;
    }

    ComponentStorage<T>& components = storage<T>();
    components.remove(entity);

    return components.emplace(entity, std::forward<Args>(args)...);
}

template<Component T>
bool Registry::remove(Entity entity)
{
    IComponentStorage* found = findStorage(typeid(T));

    if (found == nullptr)
    {
        return false;
    }

    return static_cast<ComponentStorage<T>*>(found)->remove(entity);
}

template<Component T>
T* Registry::tryGet(Entity entity)
{
    IComponentStorage* found = findStorage(typeid(T));

    if (found == nullptr)
    {
        return nullptr;
    }

    return static_cast<ComponentStorage<T>*>(found)->tryGet(entity);
}

template<Component T>
const T* Registry::tryGet(Entity entity) const
{
    const IComponentStorage* found = findStorage(typeid(T));

    if (found == nullptr)
    {
        return nullptr;
    }

    return static_cast<const ComponentStorage<T>*>(found)->tryGet(entity);
}

template<Component T>
bool Registry::has(Entity entity) const
{
    const IComponentStorage* found = findStorage(typeid(T));

    return found != nullptr && found->contains(entity);
}

template<Component T>
void Registry::reserve(usize capacity)
{
    storage<T>().reserve(capacity);
}

template<QueryComponent... Ts>
QueryView<Ts...> Registry::view()
{
    using View = QueryView<Ts...>;
    using Pools = typename View::Pools;

    return View(Pools{&storage<std::remove_const_t<Ts>>()...});
}

template<QueryComponent... Ts, typename Func>
void Registry::each(Func&& func)
{
    view<Ts...>().each(std::forward<Func>(func));
}

template<Component T>
ComponentStorage<T>& Registry::storage()
{
    const auto it = m_storages.find(typeid(T));

    if (it != m_storages.end())
    {
        return static_cast<ComponentStorage<T>&>(*it->second);
    }

    auto created = std::make_unique<ComponentStorage<T>>(m_resource);
    ComponentStorage<T>& result = *created;
    m_storages.emplace(typeid(T), std::move(created));

    return result;
}

}
