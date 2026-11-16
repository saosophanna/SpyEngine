#pragma once

#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "core/lifecycle/module.hpp"
#include "core/lifecycle/module_descriptor.hpp"
#include "core/lifecycle/module_id.hpp"
#include "core/lifecycle/module_state.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::lifecycle {

class Lifecycle;

class ModuleRegistry
{
public:
    ModuleRegistry() = default;
    ModuleRegistry(const ModuleRegistry&) = delete;
    ModuleRegistry& operator=(const ModuleRegistry&) = delete;
    ModuleRegistry(ModuleRegistry&&) = delete;
    ModuleRegistry& operator=(ModuleRegistry&&) = delete;
    ~ModuleRegistry() = default;

    template<typename ModuleType, typename... Args>
        requires std::derived_from<ModuleType, IModule>
    bool registerModule(ModuleDescriptor descriptor, Args&&... args)
    {
        return registerModule(
            std::move(descriptor), std::make_unique<ModuleType>(std::forward<Args>(args)...));
    }

    bool registerModule(ModuleDescriptor descriptor, std::unique_ptr<IModule> module);

    [[nodiscard]] bool contains(std::string_view id) const noexcept;
    [[nodiscard]] usize size() const noexcept;
    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] bool isRegistrationLocked() const noexcept;

    [[nodiscard]] IModule* find(std::string_view id) noexcept;
    [[nodiscard]] const IModule* find(std::string_view id) const noexcept;
    [[nodiscard]] const ModuleDescriptor* descriptor(std::string_view id) const noexcept;
    [[nodiscard]] std::optional<ModuleState> moduleState(std::string_view id) const noexcept;
    [[nodiscard]] std::vector<ModuleDescriptor> descriptors() const;
    [[nodiscard]] std::vector<ModuleId> registrationOrder() const;

    void lockRegistration() noexcept;

private:
    friend class Lifecycle;

    struct ModuleIdHash
    {
        using is_transparent = void;

        [[nodiscard]] std::size_t operator()(std::string_view id) const noexcept
        {
            return std::hash<std::string_view>{}(id);
        }

        [[nodiscard]] std::size_t operator()(const ModuleId& id) const noexcept
        {
            return std::hash<std::string_view>{}(id);
        }
    };

    struct ModuleIdEqual
    {
        using is_transparent = void;

        [[nodiscard]] bool operator()(std::string_view lhs, std::string_view rhs) const noexcept
        {
            return lhs == rhs;
        }
    };

    struct Entry
    {
        ModuleDescriptor descriptor;
        std::unique_ptr<IModule> instance;
        ModuleState state = ModuleState::Registered;
    };

    [[nodiscard]] Entry* findEntry(std::string_view id) noexcept;
    [[nodiscard]] const Entry* findEntry(std::string_view id) const noexcept;
    [[nodiscard]] bool setModuleState(std::string_view id, ModuleState state) noexcept;

    std::vector<std::unique_ptr<Entry>> m_entries;
    std::unordered_map<ModuleId, Entry*, ModuleIdHash, ModuleIdEqual> m_index;
    bool m_registrationLocked = false;
};

}
