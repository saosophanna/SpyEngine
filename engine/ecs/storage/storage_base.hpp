#pragma once

#include <span>

#include "core/utilities/types/types.hpp"
#include "ecs/entity/entity_id.hpp"

namespace spy::ecs {

class IComponentStorage
{
public:
    virtual ~IComponentStorage() = default;

    IComponentStorage(const IComponentStorage&) = delete;
    IComponentStorage& operator=(const IComponentStorage&) = delete;

    [[nodiscard]] virtual usize size() const noexcept = 0;
    [[nodiscard]] virtual bool contains(Entity entity) const noexcept = 0;
    [[nodiscard]] virtual std::span<const Entity> entities() const noexcept = 0;

    virtual bool remove(Entity entity) = 0;
    virtual void clear() noexcept = 0;

protected:
    IComponentStorage() = default;
};

}
