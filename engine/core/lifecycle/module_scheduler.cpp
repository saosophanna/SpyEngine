#include <algorithm>
#include <span>
#include <vector>

#include "core/lifecycle/module_scheduler.hpp"

namespace spy::lifecycle {

std::vector<ModuleId> ModuleScheduler::buildUpdateOrder(
    std::span<const ModuleDescriptor> descriptors) const
{
    std::vector<const ModuleDescriptor*> enabled;
    enabled.reserve(descriptors.size());

    for (const ModuleDescriptor& descriptor : descriptors)
    {
        if (descriptor.enabled)
        {
            enabled.push_back(&descriptor);
        }
    }

    std::stable_sort(
        enabled.begin(), enabled.end(),
        [](const ModuleDescriptor* lhs, const ModuleDescriptor* rhs)
        {
            return lhs->updateOrder < rhs->updateOrder;
        });

    std::vector<ModuleId> order;
    order.reserve(enabled.size());

    for (const ModuleDescriptor* descriptor : enabled)
    {
        order.push_back(descriptor->id);
    }

    return order;
}

}
