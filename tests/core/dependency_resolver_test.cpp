#include <cstdio>
#include <string>
#include <vector>

#include "core/lifecycle/dependency_resolver.hpp"

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
    std::string id, std::vector<std::string> dependencies = {}, bool enabled = true)
{
    return spy::lifecycle::ModuleDescriptor{std::move(id), std::move(dependencies), enabled, 0};
}

}

#define SPY_CHECK(expression) check((expression), #expression, __LINE__)

int main()
{
    using spy::lifecycle::DependencyError;
    using spy::lifecycle::DependencyResolver;
    using spy::lifecycle::ModuleDescriptor;

    const DependencyResolver resolver;

    {
        const std::vector<ModuleDescriptor> descriptors;
        const auto result = resolver.resolve(descriptors);
        SPY_CHECK(result.succeeded());
        SPY_CHECK(result.order.empty());
        SPY_CHECK(result.error == DependencyError::None);
    }

    {
        const std::vector<ModuleDescriptor> descriptors{
            makeDescriptor("Renderer", {"RHI"}),
            makeDescriptor("RHI", {"Platform"}),
            makeDescriptor("Platform")};
        const auto result = resolver.resolve(descriptors);
        const std::vector<std::string> expected{"Platform", "RHI", "Renderer"};
        SPY_CHECK(result.succeeded());
        SPY_CHECK(result.order == expected);
    }

    {
        const std::vector<ModuleDescriptor> descriptors{
            makeDescriptor("First"), makeDescriptor("Second"), makeDescriptor("Third")};
        const auto result = resolver.resolve(descriptors);
        const std::vector<std::string> expected{"First", "Second", "Third"};
        SPY_CHECK(result.succeeded());
        SPY_CHECK(result.order == expected);
    }

    {
        const std::vector<ModuleDescriptor> descriptors{
            makeDescriptor("Root"),
            makeDescriptor("Beta", {"Root"}),
            makeDescriptor("Alpha", {"Root"}),
            makeDescriptor("Leaf", {"Beta", "Alpha"})};
        const auto result = resolver.resolve(descriptors);
        const std::vector<std::string> expected{"Root", "Beta", "Alpha", "Leaf"};
        SPY_CHECK(result.succeeded());
        SPY_CHECK(result.order == expected);
    }

    {
        const std::vector<ModuleDescriptor> descriptors{
            makeDescriptor("Duplicate"), makeDescriptor("Duplicate")};
        const auto result = resolver.resolve(descriptors);
        SPY_CHECK(!result.succeeded());
        SPY_CHECK(result.error == DependencyError::DuplicateIdentifier);
        SPY_CHECK(result.failedModule == "Duplicate");
        SPY_CHECK(result.order.empty());
    }

    {
        const std::vector<ModuleDescriptor> descriptors{makeDescriptor("")};
        const auto result = resolver.resolve(descriptors);
        SPY_CHECK(!result.succeeded());
        SPY_CHECK(result.error == DependencyError::EmptyIdentifier);
    }

    {
        const std::vector<ModuleDescriptor> descriptors{makeDescriptor("Self", {"Self"})};
        const auto result = resolver.resolve(descriptors);
        SPY_CHECK(!result.succeeded());
        SPY_CHECK(result.error == DependencyError::SelfDependency);
        SPY_CHECK(result.failedModule == "Self");
    }

    {
        const std::vector<ModuleDescriptor> descriptors{
            makeDescriptor("Consumer", {"Missing"})};
        const auto result = resolver.resolve(descriptors);
        SPY_CHECK(!result.succeeded());
        SPY_CHECK(result.error == DependencyError::MissingDependency);
        SPY_CHECK(result.failedModule == "Consumer");
    }

    {
        const std::vector<ModuleDescriptor> descriptors{
            makeDescriptor("Provider", {}, false),
            makeDescriptor("Consumer", {"Provider"})};
        const auto result = resolver.resolve(descriptors);
        SPY_CHECK(!result.succeeded());
        SPY_CHECK(result.error == DependencyError::DisabledDependency);
        SPY_CHECK(result.failedModule == "Consumer");
    }

    {
        const std::vector<ModuleDescriptor> descriptors{
            makeDescriptor("Disabled", {"Missing"}, false),
            makeDescriptor("Enabled")};
        const auto result = resolver.resolve(descriptors);
        const std::vector<std::string> expected{"Enabled"};
        SPY_CHECK(result.succeeded());
        SPY_CHECK(result.order == expected);
    }

    {
        const std::vector<ModuleDescriptor> descriptors{
            makeDescriptor("A", {"B"}), makeDescriptor("B", {"C"}), makeDescriptor("C", {"A"})};
        const auto result = resolver.resolve(descriptors);
        SPY_CHECK(!result.succeeded());
        SPY_CHECK(result.error == DependencyError::CircularDependency);
        SPY_CHECK(!result.failedModule.empty());
        SPY_CHECK(result.order.empty());
    }

    {
        const std::vector<ModuleDescriptor> descriptors{
            makeDescriptor("A", {"B"}), makeDescriptor("B", {"A"}), makeDescriptor("C")};
        const auto result = resolver.resolve(descriptors);
        SPY_CHECK(!result.succeeded());
        SPY_CHECK(result.error == DependencyError::CircularDependency);
        SPY_CHECK(result.failedModule == "A");
        SPY_CHECK(result.order.empty());
    }

    {
        const std::vector<ModuleDescriptor> descriptors{
            makeDescriptor("A", {"B", "B"}), makeDescriptor("B")};
        const auto result = resolver.resolve(descriptors);
        const std::vector<std::string> expected{"B", "A"};
        SPY_CHECK(result.succeeded());
        SPY_CHECK(result.order == expected);
    }

    return g_failures == 0 ? 0 : 1;
}
