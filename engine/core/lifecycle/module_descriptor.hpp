#pragma once

#include <vector>

#include "core/lifecycle/module_id.hpp"

namespace spy::lifecycle {

struct ModuleDescriptor
{
    ModuleId id;
    std::vector<ModuleId> dependencies;
    bool enabled = true;
    int updateOrder = 0;
};

}
