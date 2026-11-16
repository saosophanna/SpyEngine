#include <functional>
#include <queue>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "core/lifecycle/dependency_resolver.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::lifecycle {

namespace {

DependencyResolution fail(
    DependencyError error, const ModuleId& failedModule)
{
    DependencyResolution resolution;
    resolution.error = error;
    resolution.failedModule = failedModule;
    return resolution;
}

}

DependencyResolution DependencyResolver::resolve(
    std::span<const ModuleDescriptor> descriptors) const
{
    std::unordered_map<ModuleId, usize> indexById;
    indexById.reserve(descriptors.size());

    for (usize index = 0; index < descriptors.size(); ++index)
    {
        const ModuleDescriptor& descriptor = descriptors[index];

        if (descriptor.id.empty())
        {
            return fail(DependencyError::EmptyIdentifier, ModuleId{});
        }

        if (!indexById.emplace(descriptor.id, index).second)
        {
            return fail(DependencyError::DuplicateIdentifier, descriptor.id);
        }
    }

    for (const ModuleDescriptor& descriptor : descriptors)
    {
        for (const ModuleId& dependency : descriptor.dependencies)
        {
            if (dependency == descriptor.id)
            {
                return fail(DependencyError::SelfDependency, descriptor.id);
            }
        }
    }

    for (const ModuleDescriptor& descriptor : descriptors)
    {
        if (!descriptor.enabled)
        {
            continue;
        }

        for (const ModuleId& dependency : descriptor.dependencies)
        {
            const auto iterator = indexById.find(dependency);

            if (iterator == indexById.end())
            {
                return fail(DependencyError::MissingDependency, descriptor.id);
            }

            if (!descriptors[iterator->second].enabled)
            {
                return fail(DependencyError::DisabledDependency, descriptor.id);
            }
        }
    }

    std::vector<usize> dependencyCount(descriptors.size(), 0);
    std::vector<std::vector<usize>> dependents(descriptors.size());
    usize enabledCount = 0;

    for (usize index = 0; index < descriptors.size(); ++index)
    {
        const ModuleDescriptor& descriptor = descriptors[index];

        if (!descriptor.enabled)
        {
            continue;
        }

        ++enabledCount;

        std::unordered_set<usize> uniqueDependencies;

        for (const ModuleId& dependency : descriptor.dependencies)
        {
            const usize dependencyIndex = indexById.find(dependency)->second;

            if (uniqueDependencies.insert(dependencyIndex).second)
            {
                ++dependencyCount[index];
                dependents[dependencyIndex].push_back(index);
            }
        }
    }

    std::priority_queue<usize, std::vector<usize>, std::greater<>> ready;

    for (usize index = 0; index < descriptors.size(); ++index)
    {
        if (descriptors[index].enabled && dependencyCount[index] == 0)
        {
            ready.push(index);
        }
    }

    DependencyResolution resolution;
    resolution.order.reserve(enabledCount);

    while (!ready.empty())
    {
        const usize index = ready.top();
        ready.pop();

        resolution.order.push_back(descriptors[index].id);

        for (const usize dependent : dependents[index])
        {
            --dependencyCount[dependent];

            if (dependencyCount[dependent] == 0)
            {
                ready.push(dependent);
            }
        }
    }

    if (resolution.order.size() != enabledCount)
    {
        ModuleId failedModule;

        for (usize index = 0; index < descriptors.size(); ++index)
        {
            if (descriptors[index].enabled && dependencyCount[index] > 0)
            {
                failedModule = descriptors[index].id;
                break;
            }
        }

        return fail(DependencyError::CircularDependency, failedModule);
    }

    return resolution;
}

}
