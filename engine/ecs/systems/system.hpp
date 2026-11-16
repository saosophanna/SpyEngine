#pragma once

#include "core/utilities/types/types.hpp"

namespace spy::ecs {

class Registry;

class ISystem
{
public:
    virtual ~ISystem() = default;

    ISystem(const ISystem&) = delete;
    ISystem& operator=(const ISystem&) = delete;

    [[nodiscard]] virtual bool update(Registry& registry, float32 deltaTime) = 0;

protected:
    ISystem() = default;
};

}
