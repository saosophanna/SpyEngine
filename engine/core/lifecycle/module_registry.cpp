#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "core/lifecycle/module_registry.hpp"

namespace spy::lifecycle {

bool ModuleRegistry::registerModule(ModuleDescriptor descriptor, std::unique_ptr<IModule> module)
{
    if (m_registrationLocked || descriptor.id.empty() || module == nullptr)
    {
        return false;
    }

    if (findEntry(descriptor.id) != nullptr)
    {
        return false;
    }

    for (const ModuleId& dependency : descriptor.dependencies)
    {
        if (dependency == descriptor.id)
        {
            return false;
        }
    }

    auto entry = std::make_unique<Entry>();
    entry->descriptor = std::move(descriptor);
    entry->instance = std::move(module);
    entry->state = entry->descriptor.enabled ? ModuleState::Registered : ModuleState::Disabled;

    Entry* pointer = entry.get();
    m_entries.push_back(std::move(entry));
    m_index.emplace(pointer->descriptor.id, pointer);

    return true;
}

bool ModuleRegistry::contains(std::string_view id) const noexcept
{
    return findEntry(id) != nullptr;
}

usize ModuleRegistry::size() const noexcept
{
    return m_entries.size();
}

bool ModuleRegistry::empty() const noexcept
{
    return m_entries.empty();
}

bool ModuleRegistry::isRegistrationLocked() const noexcept
{
    return m_registrationLocked;
}

IModule* ModuleRegistry::find(std::string_view id) noexcept
{
    Entry* entry = findEntry(id);
    return entry == nullptr ? nullptr : entry->instance.get();
}

const IModule* ModuleRegistry::find(std::string_view id) const noexcept
{
    const Entry* entry = findEntry(id);
    return entry == nullptr ? nullptr : entry->instance.get();
}

const ModuleDescriptor* ModuleRegistry::descriptor(std::string_view id) const noexcept
{
    const Entry* entry = findEntry(id);
    return entry == nullptr ? nullptr : &entry->descriptor;
}

std::optional<ModuleState> ModuleRegistry::moduleState(std::string_view id) const noexcept
{
    const Entry* entry = findEntry(id);
    if (entry == nullptr)
    {
        return std::nullopt;
    }

    return entry->state;
}

std::vector<ModuleDescriptor> ModuleRegistry::descriptors() const
{
    std::vector<ModuleDescriptor> result;
    result.reserve(m_entries.size());

    for (const std::unique_ptr<Entry>& entry : m_entries)
    {
        result.push_back(entry->descriptor);
    }

    return result;
}

std::vector<ModuleId> ModuleRegistry::registrationOrder() const
{
    std::vector<ModuleId> result;
    result.reserve(m_entries.size());

    for (const std::unique_ptr<Entry>& entry : m_entries)
    {
        result.push_back(entry->descriptor.id);
    }

    return result;
}

void ModuleRegistry::lockRegistration() noexcept
{
    m_registrationLocked = true;
}

ModuleRegistry::Entry* ModuleRegistry::findEntry(std::string_view id) noexcept
{
    const auto iterator = m_index.find(id);
    return iterator == m_index.end() ? nullptr : iterator->second;
}

const ModuleRegistry::Entry* ModuleRegistry::findEntry(std::string_view id) const noexcept
{
    const auto iterator = m_index.find(id);
    return iterator == m_index.end() ? nullptr : iterator->second;
}

bool ModuleRegistry::setModuleState(std::string_view id, ModuleState state) noexcept
{
    Entry* entry = findEntry(id);
    if (entry == nullptr || !canTransitionModuleState(entry->state, state))
    {
        return false;
    }

    entry->state = state;
    return true;
}

}
