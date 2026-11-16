#pragma once

namespace spy::lifecycle {

enum class ModuleState
{
    Registered,
    Disabled,
    WaitingForDependencies,
    Initializing,
    Initialized,
    Running,
    Stopping,
    Stopped,
    Failed
};

[[nodiscard]] constexpr bool canTransitionModuleState(ModuleState from, ModuleState to) noexcept
{
    switch (from)
    {
    case ModuleState::Registered:
        return to == ModuleState::Disabled || to == ModuleState::WaitingForDependencies
            || to == ModuleState::Failed;
    case ModuleState::Disabled:
        return false;
    case ModuleState::WaitingForDependencies:
        return to == ModuleState::Initializing || to == ModuleState::Failed;
    case ModuleState::Initializing:
        return to == ModuleState::Initialized || to == ModuleState::Failed;
    case ModuleState::Initialized:
        return to == ModuleState::Running || to == ModuleState::Stopping
            || to == ModuleState::Failed;
    case ModuleState::Running:
        return to == ModuleState::Stopping || to == ModuleState::Failed;
    case ModuleState::Stopping:
        return to == ModuleState::Stopped || to == ModuleState::Failed;
    case ModuleState::Stopped:
    case ModuleState::Failed:
        return false;
    }

    return false;
}

}
