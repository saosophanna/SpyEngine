#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "core/lifecycle/lifecycle.hpp"

namespace {

int g_failures = 0;

void check(bool condition, const char* expression, int line)
{
    if (!condition)
    {
        std::fprintf(stderr, "FAILED: %s (line %d)\n", expression, line);
        ++g_failures;
    }
}

}

#define SPY_CHECK(expression) check((expression), #expression, __LINE__)

namespace {

using spy::lifecycle::IModule;
using spy::lifecycle::Lifecycle;
using spy::lifecycle::LifecycleError;
using spy::lifecycle::LifecycleState;
using spy::lifecycle::ModuleDescriptor;
using spy::lifecycle::ModuleState;

class TrackingModule : public IModule
{
public:
    TrackingModule(
        std::vector<std::string>& events, std::string name, bool initializeResult,
        bool throwOnInitialize, bool throwOnUpdate, bool throwOnShutdown)
        : m_events(events)
        , m_name(std::move(name))
        , m_initializeResult(initializeResult)
        , m_throwOnInitialize(throwOnInitialize)
        , m_throwOnUpdate(throwOnUpdate)
        , m_throwOnShutdown(throwOnShutdown)
    {
    }

    bool initialize() override
    {
        m_events.push_back("init:" + m_name);

        if (m_throwOnInitialize)
        {
            throw std::runtime_error("initialize");
        }

        return m_initializeResult;
    }

    void update(spy::float64 deltaTime) override
    {
        ++m_updateCount;
        m_receivedDeltaTime = deltaTime;
        m_events.push_back("update:" + m_name);

        if (m_throwOnUpdate)
        {
            throw std::runtime_error("update");
        }
    }

    void shutdown() override
    {
        m_events.push_back("shutdown:" + m_name);

        if (m_throwOnShutdown)
        {
            throw std::runtime_error("shutdown");
        }
    }

    [[nodiscard]] int updateCount() const noexcept
    {
        return m_updateCount;
    }

    [[nodiscard]] spy::float64 receivedDeltaTime() const noexcept
    {
        return m_receivedDeltaTime;
    }

private:
    std::vector<std::string>& m_events;
    std::string m_name;
    bool m_initializeResult = true;
    bool m_throwOnInitialize = false;
    bool m_throwOnUpdate = false;
    bool m_throwOnShutdown = false;
    int m_updateCount = 0;
    spy::float64 m_receivedDeltaTime = 0.0;
};

TrackingModule* findModule(Lifecycle& lifecycle, const std::string& id)
{
    return dynamic_cast<TrackingModule*>(lifecycle.registry().find(id));
}

void addModule(
    Lifecycle& lifecycle, std::vector<std::string>& events, const std::string& id,
    std::vector<std::string> dependencies = {}, int updateOrder = 0, bool enabled = true,
    bool initializeResult = true, bool throwOnInitialize = false, bool throwOnUpdate = false,
    bool throwOnShutdown = false)
{
    auto module = std::make_unique<TrackingModule>(
        events, id, initializeResult, throwOnInitialize, throwOnUpdate, throwOnShutdown);

    SPY_CHECK(lifecycle.registry().registerModule(
        ModuleDescriptor{id, std::move(dependencies), enabled, updateOrder}, std::move(module)));
}

void testSuccessfulLifecycle()
{
    std::vector<std::string> events;
    Lifecycle lifecycle;
    addModule(lifecycle, events, "Renderer", {"RHI"}, 0);
    addModule(lifecycle, events, "Platform", {}, -100);
    addModule(lifecycle, events, "RHI", {"Platform"}, -90);

    lifecycle.update(0.5);
    SPY_CHECK(lifecycle.state() == LifecycleState::Uninitialized);
    SPY_CHECK(events.empty());

    SPY_CHECK(lifecycle.initialize());
    SPY_CHECK(lifecycle.isRunning());
    SPY_CHECK(lifecycle.state() == LifecycleState::Running);
    SPY_CHECK(lifecycle.lastError() == LifecycleError::None);
    SPY_CHECK(lifecycle.lastDependencyError() == spy::lifecycle::DependencyError::None);
    SPY_CHECK(lifecycle.lastErrorModule().empty());

    const std::vector<std::string> expectedInitialization{"Platform", "RHI", "Renderer"};
    SPY_CHECK(lifecycle.initializationOrder() == expectedInitialization);

    const std::vector<std::string> expectedUpdate{"Platform", "RHI", "Renderer"};
    SPY_CHECK(lifecycle.updateOrder() == expectedUpdate);

    const std::vector<std::string> expectedEvents{
        "init:Platform", "init:RHI", "init:Renderer"};
    SPY_CHECK(events == expectedEvents);

    SPY_CHECK(lifecycle.moduleState("Platform") == ModuleState::Running);
    SPY_CHECK(lifecycle.moduleState("RHI") == ModuleState::Running);
    SPY_CHECK(lifecycle.moduleState("Renderer") == ModuleState::Running);

    lifecycle.update(0.016);

    const std::vector<std::string> expectedAfterUpdate{
        "init:Platform", "init:RHI", "init:Renderer",
        "update:Platform", "update:RHI", "update:Renderer"};
    SPY_CHECK(events == expectedAfterUpdate);

    TrackingModule* platform = findModule(lifecycle, "Platform");
    SPY_CHECK(platform != nullptr);
    SPY_CHECK(platform != nullptr && platform->updateCount() == 1);
    SPY_CHECK(platform != nullptr && platform->receivedDeltaTime() == 0.016);

    SPY_CHECK(!lifecycle.registry().registerModule(
        ModuleDescriptor{"Late", {}, true, 0},
        std::make_unique<TrackingModule>(events, "Late", true, false, false, false)));

    SPY_CHECK(!lifecycle.initialize());
    SPY_CHECK(lifecycle.lastError() == LifecycleError::InvalidState);
    SPY_CHECK(lifecycle.state() == LifecycleState::Running);

    lifecycle.shutdown();
    SPY_CHECK(lifecycle.state() == LifecycleState::Stopped);
    SPY_CHECK(!lifecycle.isRunning());

    const std::vector<std::string> expectedAfterShutdown{
        "init:Platform", "init:RHI", "init:Renderer",
        "update:Platform", "update:RHI", "update:Renderer",
        "shutdown:Renderer", "shutdown:RHI", "shutdown:Platform"};
    SPY_CHECK(events == expectedAfterShutdown);

    SPY_CHECK(lifecycle.moduleState("Platform") == ModuleState::Stopped);
    SPY_CHECK(lifecycle.moduleState("RHI") == ModuleState::Stopped);
    SPY_CHECK(lifecycle.moduleState("Renderer") == ModuleState::Stopped);

    lifecycle.update(0.016);
    SPY_CHECK(events == expectedAfterShutdown);

    lifecycle.shutdown();
    SPY_CHECK(events == expectedAfterShutdown);

    SPY_CHECK(!lifecycle.initialize());
    SPY_CHECK(lifecycle.lastError() == LifecycleError::InvalidState);
}

void testInitializationFailureRollsBack()
{
    std::vector<std::string> events;
    Lifecycle lifecycle;
    addModule(lifecycle, events, "Platform");
    addModule(lifecycle, events, "RHI", {"Platform"});
    addModule(lifecycle, events, "Renderer", {"RHI"}, 0, true, false);
    addModule(lifecycle, events, "World", {"Renderer"});

    SPY_CHECK(!lifecycle.initialize());
    SPY_CHECK(lifecycle.state() == LifecycleState::Failed);
    SPY_CHECK(lifecycle.lastError() == LifecycleError::ModuleInitializationFailed);
    SPY_CHECK(lifecycle.lastErrorModule() == "Renderer");

    const std::vector<std::string> expectedEvents{
        "init:Platform", "init:RHI", "init:Renderer", "shutdown:RHI", "shutdown:Platform"};
    SPY_CHECK(events == expectedEvents);

    SPY_CHECK(lifecycle.moduleState("Platform") == ModuleState::Stopped);
    SPY_CHECK(lifecycle.moduleState("RHI") == ModuleState::Stopped);
    SPY_CHECK(lifecycle.moduleState("Renderer") == ModuleState::Failed);
    SPY_CHECK(lifecycle.moduleState("World") == ModuleState::WaitingForDependencies);

    lifecycle.shutdown();
    SPY_CHECK(lifecycle.state() == LifecycleState::Stopped);
    SPY_CHECK(events == expectedEvents);

    SPY_CHECK(!lifecycle.initialize());
    SPY_CHECK(lifecycle.lastError() == LifecycleError::InvalidState);
}

void testInitializationExceptionRollsBack()
{
    std::vector<std::string> events;
    Lifecycle lifecycle;
    addModule(lifecycle, events, "Platform");
    addModule(lifecycle, events, "RHI", {"Platform"}, 0, true, true, true);

    SPY_CHECK(!lifecycle.initialize());
    SPY_CHECK(lifecycle.state() == LifecycleState::Failed);
    SPY_CHECK(lifecycle.lastError() == LifecycleError::ModuleInitializationFailed);
    SPY_CHECK(lifecycle.lastErrorModule() == "RHI");

    const std::vector<std::string> expectedEvents{
        "init:Platform", "init:RHI", "shutdown:Platform"};
    SPY_CHECK(events == expectedEvents);

    SPY_CHECK(lifecycle.moduleState("Platform") == ModuleState::Stopped);
    SPY_CHECK(lifecycle.moduleState("RHI") == ModuleState::Failed);
}

void testDependencyResolutionFailure()
{
    std::vector<std::string> events;
    Lifecycle lifecycle;
    addModule(lifecycle, events, "Consumer", {"Missing"});

    SPY_CHECK(!lifecycle.initialize());
    SPY_CHECK(lifecycle.state() == LifecycleState::Failed);
    SPY_CHECK(lifecycle.lastError() == LifecycleError::DependencyResolutionFailed);
    SPY_CHECK(
        lifecycle.lastDependencyError() == spy::lifecycle::DependencyError::MissingDependency);
    SPY_CHECK(lifecycle.lastErrorModule() == "Consumer");
    SPY_CHECK(events.empty());
    SPY_CHECK(lifecycle.moduleState("Consumer") == ModuleState::Registered);
}

void testDisabledDependencyFailure()
{
    std::vector<std::string> events;
    Lifecycle lifecycle;
    addModule(lifecycle, events, "Provider", {}, 0, false);
    addModule(lifecycle, events, "Consumer", {"Provider"});

    SPY_CHECK(!lifecycle.initialize());
    SPY_CHECK(lifecycle.state() == LifecycleState::Failed);
    SPY_CHECK(lifecycle.lastError() == LifecycleError::DependencyResolutionFailed);
    SPY_CHECK(
        lifecycle.lastDependencyError() == spy::lifecycle::DependencyError::DisabledDependency);
    SPY_CHECK(lifecycle.lastErrorModule() == "Consumer");
    SPY_CHECK(events.empty());
}

void testDisabledModuleIsSkipped()
{
    std::vector<std::string> events;
    Lifecycle lifecycle;
    addModule(lifecycle, events, "Enabled", {}, 0);
    addModule(lifecycle, events, "DisabledModule", {}, -100, false, false, true, true, true);

    SPY_CHECK(lifecycle.initialize());
    SPY_CHECK(lifecycle.state() == LifecycleState::Running);
    SPY_CHECK(lifecycle.moduleState("DisabledModule") == ModuleState::Disabled);

    const std::vector<std::string> expectedUpdateOrder{"Enabled"};
    SPY_CHECK(lifecycle.updateOrder() == expectedUpdateOrder);

    lifecycle.update(0.016);

    const std::vector<std::string> expectedAfterUpdate{
        "init:Enabled", "update:Enabled"};
    SPY_CHECK(events == expectedAfterUpdate);

    lifecycle.shutdown();

    const std::vector<std::string> expectedAfterShutdown{
        "init:Enabled", "update:Enabled", "shutdown:Enabled"};
    SPY_CHECK(events == expectedAfterShutdown);
    SPY_CHECK(lifecycle.moduleState("DisabledModule") == ModuleState::Disabled);
}

void testUpdateOrderIsIndependentOfInitializationOrder()
{
    std::vector<std::string> events;
    Lifecycle lifecycle;
    addModule(lifecycle, events, "First", {}, 10);
    addModule(lifecycle, events, "Second", {}, -10);

    SPY_CHECK(lifecycle.initialize());

    const std::vector<std::string> expectedInitialization{"First", "Second"};
    SPY_CHECK(lifecycle.initializationOrder() == expectedInitialization);

    const std::vector<std::string> expectedUpdate{"Second", "First"};
    SPY_CHECK(lifecycle.updateOrder() == expectedUpdate);
}

void testUpdateFailureSkipsFailedModule()
{
    std::vector<std::string> events;
    Lifecycle lifecycle;
    addModule(lifecycle, events, "Broken", {}, 0, true, true, false, true);
    addModule(lifecycle, events, "Healthy", {}, 1);

    SPY_CHECK(lifecycle.initialize());

    lifecycle.update(0.1);
    SPY_CHECK(lifecycle.state() == LifecycleState::Running);
    SPY_CHECK(lifecycle.lastError() == LifecycleError::ModuleUpdateFailed);
    SPY_CHECK(lifecycle.lastErrorModule() == "Broken");
    SPY_CHECK(lifecycle.moduleState("Broken") == ModuleState::Failed);
    SPY_CHECK(lifecycle.moduleState("Healthy") == ModuleState::Running);

    TrackingModule* broken = findModule(lifecycle, "Broken");
    TrackingModule* healthy = findModule(lifecycle, "Healthy");
    SPY_CHECK(broken != nullptr && broken->updateCount() == 1);
    SPY_CHECK(healthy != nullptr && healthy->updateCount() == 1);

    lifecycle.update(0.2);
    SPY_CHECK(broken != nullptr && broken->updateCount() == 1);
    SPY_CHECK(healthy != nullptr && healthy->updateCount() == 2);

    lifecycle.shutdown();
    SPY_CHECK(lifecycle.moduleState("Broken") == ModuleState::Failed);
    SPY_CHECK(lifecycle.moduleState("Healthy") == ModuleState::Stopped);
}

void testShutdownExceptionContinues()
{
    std::vector<std::string> events;
    Lifecycle lifecycle;
    addModule(lifecycle, events, "First");
    addModule(lifecycle, events, "Second", {}, 0, true, true, false, false, true);

    SPY_CHECK(lifecycle.initialize());
    lifecycle.shutdown();

    SPY_CHECK(lifecycle.state() == LifecycleState::Stopped);
    SPY_CHECK(lifecycle.lastError() == LifecycleError::ModuleShutdownFailed);
    SPY_CHECK(lifecycle.lastErrorModule() == "Second");
    SPY_CHECK(lifecycle.moduleState("First") == ModuleState::Stopped);
    SPY_CHECK(lifecycle.moduleState("Second") == ModuleState::Stopped);

    const std::vector<std::string> expectedEvents{
        "init:First", "init:Second", "shutdown:Second", "shutdown:First"};
    SPY_CHECK(events == expectedEvents);
}

void testEmptyLifecycle()
{
    Lifecycle lifecycle;

    SPY_CHECK(lifecycle.initialize());
    SPY_CHECK(lifecycle.state() == LifecycleState::Running);
    SPY_CHECK(lifecycle.initializationOrder().empty());
    SPY_CHECK(lifecycle.updateOrder().empty());

    lifecycle.update(0.016);
    lifecycle.shutdown();
    SPY_CHECK(lifecycle.state() == LifecycleState::Stopped);
}

void testModuleStateTransitions()
{
    using spy::lifecycle::canTransitionModuleState;

    SPY_CHECK(canTransitionModuleState(
        ModuleState::Registered, ModuleState::WaitingForDependencies));
    SPY_CHECK(canTransitionModuleState(
        ModuleState::WaitingForDependencies, ModuleState::Initializing));
    SPY_CHECK(canTransitionModuleState(ModuleState::Initializing, ModuleState::Initialized));
    SPY_CHECK(canTransitionModuleState(ModuleState::Initialized, ModuleState::Running));
    SPY_CHECK(canTransitionModuleState(ModuleState::Running, ModuleState::Stopping));
    SPY_CHECK(canTransitionModuleState(ModuleState::Stopping, ModuleState::Stopped));
    SPY_CHECK(canTransitionModuleState(ModuleState::Registered, ModuleState::Disabled));

    SPY_CHECK(!canTransitionModuleState(ModuleState::Stopped, ModuleState::Running));
    SPY_CHECK(!canTransitionModuleState(ModuleState::Disabled, ModuleState::Initializing));
    SPY_CHECK(!canTransitionModuleState(ModuleState::Failed, ModuleState::Stopping));
    SPY_CHECK(!canTransitionModuleState(ModuleState::Initializing, ModuleState::Running));
}

}

int main()
{
    testSuccessfulLifecycle();
    testInitializationFailureRollsBack();
    testInitializationExceptionRollsBack();
    testDependencyResolutionFailure();
    testDisabledDependencyFailure();
    testDisabledModuleIsSkipped();
    testUpdateOrderIsIndependentOfInitializationOrder();
    testUpdateFailureSkipsFailedModule();
    testShutdownExceptionContinues();
    testEmptyLifecycle();
    testModuleStateTransitions();

    return g_failures == 0 ? 0 : 1;
}
