#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "core/lifecycle/dependency_resolver.hpp"
#include "core/lifecycle/module_registry.hpp"
#include "core/lifecycle/module_scheduler.hpp"
#include "core/lifecycle/module_state.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::lifecycle {

enum class LifecycleState
{
    Uninitialized,
    Initializing,
    Running,
    Stopping,
    Stopped,
    Failed
};

enum class LifecycleError
{
    None,
    InvalidState,
    DependencyResolutionFailed,
    ModuleInitializationFailed,
    ModuleUpdateFailed,
    ModuleShutdownFailed,
    InvalidModuleTransition
};

class Lifecycle
{
public:
    Lifecycle() = default;
    Lifecycle(const Lifecycle&) = delete;
    Lifecycle& operator=(const Lifecycle&) = delete;
    Lifecycle(Lifecycle&&) = delete;
    Lifecycle& operator=(Lifecycle&&) = delete;
    ~Lifecycle();

    [[nodiscard]] ModuleRegistry& registry() noexcept;
    [[nodiscard]] const ModuleRegistry& registry() const noexcept;

    bool initialize();
    void update(float64 deltaTime);
    void shutdown();

    [[nodiscard]] bool isRunning() const noexcept;
    [[nodiscard]] LifecycleState state() const noexcept;
    [[nodiscard]] std::optional<ModuleState> moduleState(std::string_view moduleId) const;
    [[nodiscard]] LifecycleError lastError() const noexcept;
    [[nodiscard]] DependencyError lastDependencyError() const noexcept;
    [[nodiscard]] const ModuleId& lastErrorModule() const noexcept;
    [[nodiscard]] const std::vector<ModuleId>& initializationOrder() const noexcept;
    [[nodiscard]] const std::vector<ModuleId>& updateOrder() const noexcept;

private:
    void recordError(LifecycleError error, std::string_view moduleId = {});
    void shutdownInitializedModules();
    void clearExecutionState() noexcept;

    ModuleRegistry m_registry;
    DependencyResolver m_resolver;
    ModuleScheduler m_scheduler;

    LifecycleState m_state = LifecycleState::Uninitialized;
    LifecycleError m_lastError = LifecycleError::None;
    DependencyError m_lastDependencyError = DependencyError::None;
    ModuleId m_lastErrorModule;

    std::vector<ModuleId> m_initializationOrder;
    std::vector<ModuleId> m_updateOrder;
    std::vector<ModuleRegistry::Entry*> m_initializedModules;
    std::vector<ModuleRegistry::Entry*> m_updateModules;
};

}
