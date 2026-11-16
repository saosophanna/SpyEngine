#include <cstdio>
#include <string>
#include <vector>

#include "core/lifecycle/module_scheduler.hpp"

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

spy::lifecycle::ModuleDescriptor makeDescriptor(
    std::string id, int updateOrder, bool enabled = true)
{
    return spy::lifecycle::ModuleDescriptor{
        std::move(id), std::vector<std::string>{}, enabled, updateOrder};
}

}

#define SPY_CHECK(expression) check((expression), #expression, __LINE__)

int main()
{
    using spy::lifecycle::ModuleDescriptor;
    using spy::lifecycle::ModuleScheduler;

    const ModuleScheduler scheduler;

    {
        const std::vector<ModuleDescriptor> descriptors;
        SPY_CHECK(scheduler.buildUpdateOrder(descriptors).empty());
    }

    {
        const std::vector<ModuleDescriptor> descriptors{
            makeDescriptor("Renderer", 0),
            makeDescriptor("Platform", -100),
            makeDescriptor("Audio", 10),
            makeDescriptor("RHI", -90)};
        const std::vector<std::string> expected{"Platform", "RHI", "Renderer", "Audio"};
        SPY_CHECK(scheduler.buildUpdateOrder(descriptors) == expected);
    }

    {
        const std::vector<ModuleDescriptor> descriptors{
            makeDescriptor("First", 5),
            makeDescriptor("Second", 5),
            makeDescriptor("Third", 5)};
        const std::vector<std::string> expected{"First", "Second", "Third"};
        SPY_CHECK(scheduler.buildUpdateOrder(descriptors) == expected);
    }

    {
        const std::vector<ModuleDescriptor> descriptors{
            makeDescriptor("EnabledFirst", 1),
            makeDescriptor("Disabled", -100, false),
            makeDescriptor("EnabledSecond", 2)};
        const std::vector<std::string> expected{"EnabledFirst", "EnabledSecond"};
        SPY_CHECK(scheduler.buildUpdateOrder(descriptors) == expected);
    }

    {
        const std::vector<ModuleDescriptor> descriptors{makeDescriptor("Disabled", 0, false)};
        SPY_CHECK(scheduler.buildUpdateOrder(descriptors).empty());
    }

    return g_failures == 0 ? 0 : 1;
}
