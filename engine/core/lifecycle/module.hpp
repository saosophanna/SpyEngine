#pragma once

#include "core/utilities/types/types.hpp"

namespace spy::lifecycle {

class IModule
{
public:
    virtual ~IModule() = default;

    IModule(const IModule&) = delete;
    IModule& operator=(const IModule&) = delete;

    [[nodiscard]] virtual bool initialize() = 0;

    virtual void update(float64 deltaTime)
    {
        (void)deltaTime;
    }

    virtual void shutdown() = 0;

protected:
    IModule() = default;
};

}
