#pragma once

#include <span>
#include <vector>

#include "core/lifecycle/module_descriptor.hpp"
#include "core/lifecycle/module_id.hpp"

namespace spy::lifecycle {

class ModuleScheduler
{
public:
    [[nodiscard]] std::vector<ModuleId> buildUpdateOrder(
        std::span<const ModuleDescriptor> descriptors) const;
};

}
