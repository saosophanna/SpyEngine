#pragma once

#include <span>
#include <vector>

#include "core/lifecycle/module_descriptor.hpp"
#include "core/lifecycle/module_id.hpp"

namespace spy::lifecycle {

enum class DependencyError
{
    None,
    EmptyIdentifier,
    DuplicateIdentifier,
    SelfDependency,
    MissingDependency,
    DisabledDependency,
    CircularDependency
};

struct DependencyResolution
{
    std::vector<ModuleId> order;
    DependencyError error = DependencyError::None;
    ModuleId failedModule;

    [[nodiscard]] bool succeeded() const noexcept
    {
        return error == DependencyError::None;
    }
};

class DependencyResolver
{
public:
    [[nodiscard]] DependencyResolution resolve(
        std::span<const ModuleDescriptor> descriptors) const;
};

}
