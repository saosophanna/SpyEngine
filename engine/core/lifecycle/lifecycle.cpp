#include <string_view>
#include <vector>

#include "core/lifecycle/lifecycle.hpp"

namespace spy::lifecycle {

Lifecycle::~Lifecycle()
{
    shutdown();
}

ModuleRegistry& Lifecycle::registry() noexcept
{
    return m_registry;
}

const ModuleRegistry& Lifecycle::registry() const noexcept
{
    return m_registry;
}

bool Lifecycle::initialize()
{
    if (m_state != LifecycleState::Uninitialized)
    {
        recordError(LifecycleError::InvalidState);
        return false;
    }

    m_registry.lockRegistration();

    const std::vector<ModuleDescriptor> descriptors = m_registry.descriptors();
    const DependencyResolution resolution = m_resolver.resolve(descriptors);

    if (!resolution.succeeded())
    {
        m_lastDependencyError = resolution.error;
        recordError(LifecycleError::DependencyResolutionFailed, resolution.failedModule);
        m_state = LifecycleState::Failed;
        return false;
    }

    m_initializationOrder = resolution.order;
    m_updateOrder = m_scheduler.buildUpdateOrder(descriptors);
    m_state = LifecycleState::Initializing;

    for (const ModuleId& moduleId : m_initializationOrder)
    {
        if (!m_registry.setModuleState(moduleId, ModuleState::WaitingForDependencies))
        {
            recordError(LifecycleError::InvalidModuleTransition, moduleId);
            m_state = LifecycleState::Failed;
            return false;
        }
    }

    for (const ModuleId& moduleId : m_initializationOrder)
    {
        ModuleRegistry::Entry* entry = m_registry.findEntry(moduleId);

        if (entry == nullptr
            || !m_registry.setModuleState(moduleId, ModuleState::Initializing))
        {
            recordError(LifecycleError::InvalidModuleTransition, moduleId);
            m_state = LifecycleState::Failed;
            shutdownInitializedModules();
            return false;
        }

        bool initialized = false;

        try
        {
            initialized = entry->instance->initialize();
        }
        catch (...)
        {
            initialized = false;
        }

        if (!initialized)
        {
            (void)m_registry.setModuleState(moduleId, ModuleState::Failed);
            recordError(LifecycleError::ModuleInitializationFailed, moduleId);
            m_state = LifecycleState::Failed;
            shutdownInitializedModules();
            return false;
        }

        (void)m_registry.setModuleState(moduleId, ModuleState::Initialized);
        m_initializedModules.push_back(entry);
    }

    for (const ModuleId& moduleId : m_updateOrder)
    {
        ModuleRegistry::Entry* entry = m_registry.findEntry(moduleId);

        if (entry == nullptr
            || !m_registry.setModuleState(moduleId, ModuleState::Running))
        {
            recordError(LifecycleError::InvalidModuleTransition, moduleId);
            m_state = LifecycleState::Failed;
            shutdownInitializedModules();
            return false;
        }

        m_updateModules.push_back(entry);
    }

    m_state = LifecycleState::Running;
    m_lastError = LifecycleError::None;
    m_lastDependencyError = DependencyError::None;
    m_lastErrorModule.clear();

    return true;
}

void Lifecycle::update(float64 deltaTime)
{
    if (m_state != LifecycleState::Running)
    {
        return;
    }

    for (ModuleRegistry::Entry* entry : m_updateModules)
    {
        if (entry->state != ModuleState::Running)
        {
            continue;
        }

        try
        {
            entry->instance->update(deltaTime);
        }
        catch (...)
        {
            (void)m_registry.setModuleState(entry->descriptor.id, ModuleState::Failed);
            recordError(LifecycleError::ModuleUpdateFailed, entry->descriptor.id);
        }
    }
}

void Lifecycle::shutdown()
{
    if (m_state == LifecycleState::Uninitialized || m_state == LifecycleState::Stopped)
    {
        return;
    }

    m_state = LifecycleState::Stopping;
    shutdownInitializedModules();
    clearExecutionState();
    m_state = LifecycleState::Stopped;
}

bool Lifecycle::isRunning() const noexcept
{
    return m_state == LifecycleState::Running;
}

LifecycleState Lifecycle::state() const noexcept
{
    return m_state;
}

std::optional<ModuleState> Lifecycle::moduleState(std::string_view moduleId) const
{
    return m_registry.moduleState(moduleId);
}

LifecycleError Lifecycle::lastError() const noexcept
{
    return m_lastError;
}

DependencyError Lifecycle::lastDependencyError() const noexcept
{
    return m_lastDependencyError;
}

const ModuleId& Lifecycle::lastErrorModule() const noexcept
{
    return m_lastErrorModule;
}

const std::vector<ModuleId>& Lifecycle::initializationOrder() const noexcept
{
    return m_initializationOrder;
}

const std::vector<ModuleId>& Lifecycle::updateOrder() const noexcept
{
    return m_updateOrder;
}

void Lifecycle::recordError(LifecycleError error, std::string_view moduleId)
{
    m_lastError = error;

    if (moduleId.empty())
    {
        m_lastErrorModule.clear();
        return;
    }

    m_lastErrorModule.assign(moduleId);
}

void Lifecycle::shutdownInitializedModules()
{
    for (auto iterator = m_initializedModules.rbegin();
         iterator != m_initializedModules.rend();
         ++iterator)
    {
        ModuleRegistry::Entry* entry = *iterator;

        const bool stopping =
            m_registry.setModuleState(entry->descriptor.id, ModuleState::Stopping);

        try
        {
            entry->instance->shutdown();
        }
        catch (...)
        {
            if (m_lastError == LifecycleError::None)
            {
                recordError(LifecycleError::ModuleShutdownFailed, entry->descriptor.id);
            }
        }

        if (stopping)
        {
            (void)m_registry.setModuleState(entry->descriptor.id, ModuleState::Stopped);
        }
    }

    m_initializedModules.clear();
    m_updateModules.clear();
}

void Lifecycle::clearExecutionState() noexcept
{
    m_initializationOrder.clear();
    m_updateOrder.clear();
    m_updateModules.clear();
}

}
