#include <cstdio>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "core/lifecycle/module_registry.hpp"

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

class DummyModule : public spy::lifecycle::IModule
{
public:
    bool initialize() override
    {
        return true;
    }

    void shutdown() override
    {
    }
};

class ParameterModule : public spy::lifecycle::IModule
{
public:
    explicit ParameterModule(int value) : m_value(value)
    {
    }

    bool initialize() override
    {
        return true;
    }

    void shutdown() override
    {
    }

    [[nodiscard]] int value() const noexcept
    {
        return m_value;
    }

private:
    int m_value = 0;
};

spy::lifecycle::ModuleDescriptor makeDescriptor(
    std::string id, std::vector<std::string> dependencies = {}, bool enabled = true,
    int updateOrder = 0)
{
    return spy::lifecycle::ModuleDescriptor{
        std::move(id), std::move(dependencies), enabled, updateOrder};
}

}

#define SPY_CHECK(expression) check((expression), #expression, __LINE__)

int main()
{
    using spy::lifecycle::ModuleRegistry;
    using spy::lifecycle::ModuleState;

    ModuleRegistry registry;
    SPY_CHECK(registry.empty());
    SPY_CHECK(registry.size() == 0);
    SPY_CHECK(!registry.isRegistrationLocked());
    SPY_CHECK(!registry.contains("Alpha"));
    SPY_CHECK(registry.find("Alpha") == nullptr);
    SPY_CHECK(registry.descriptor("Alpha") == nullptr);
    SPY_CHECK(!registry.moduleState("Alpha").has_value());

    SPY_CHECK(registry.registerModule<DummyModule>(makeDescriptor("Alpha")));
    SPY_CHECK(!registry.empty());
    SPY_CHECK(registry.size() == 1);
    SPY_CHECK(registry.contains("Alpha"));
    SPY_CHECK(registry.find("Alpha") != nullptr);
    SPY_CHECK(registry.moduleState("Alpha") == ModuleState::Registered);
    SPY_CHECK(registry.descriptor("Alpha") != nullptr);
    SPY_CHECK(registry.descriptor("Alpha")->id == "Alpha");

    SPY_CHECK(!registry.registerModule<DummyModule>(makeDescriptor("Alpha")));
    SPY_CHECK(registry.size() == 1);

    SPY_CHECK(!registry.registerModule<DummyModule>(makeDescriptor("")));
    SPY_CHECK(!registry.registerModule(
        makeDescriptor("Null"), std::unique_ptr<spy::lifecycle::IModule>{}));
    SPY_CHECK(!registry.registerModule<DummyModule>(makeDescriptor("Self", {"Self"})));

    SPY_CHECK(registry.registerModule<ParameterModule>(makeDescriptor("Parameter"), 42));
    auto* parameter = dynamic_cast<ParameterModule*>(registry.find("Parameter"));
    SPY_CHECK(parameter != nullptr);
    SPY_CHECK(parameter != nullptr && parameter->value() == 42);

    SPY_CHECK(registry.registerModule<DummyModule>(makeDescriptor("Disabled", {}, false)));
    SPY_CHECK(registry.moduleState("Disabled") == ModuleState::Disabled);
    SPY_CHECK(!registry.isRegistrationLocked());

    const std::vector<std::string> expectedOrder{"Alpha", "Parameter", "Disabled"};
    SPY_CHECK(registry.registrationOrder() == expectedOrder);

    const std::vector<spy::lifecycle::ModuleDescriptor> descriptors = registry.descriptors();
    SPY_CHECK(descriptors.size() == 3);
    SPY_CHECK(descriptors.size() == 3 && descriptors[0].id == "Alpha");
    SPY_CHECK(descriptors.size() == 3 && descriptors[2].id == "Disabled");

    registry.lockRegistration();
    SPY_CHECK(registry.isRegistrationLocked());
    SPY_CHECK(!registry.registerModule<DummyModule>(makeDescriptor("Late")));
    SPY_CHECK(registry.size() == 3);

    return g_failures == 0 ? 0 : 1;
}
