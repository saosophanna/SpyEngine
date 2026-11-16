#include "core/events/detail/event_registry.hpp"

#include <cstddef>

namespace spy::events::detail {

std::size_t EventRegistry::subscriptionCount() const
{
    std::size_t count = 0;

    for (const auto& entry : m_channels)
    {
        count += entry.second->size();
    }

    return count;
}

std::size_t EventRegistry::eventTypeCount() const
{
    std::size_t count = 0;

    for (const auto& entry : m_channels)
    {
        if (entry.second->size() > 0)
        {
            ++count;
        }
    }

    return count;
}

std::size_t EventRegistry::dispatchDepth() const noexcept
{
    return m_dispatchDepth;
}

void EventRegistry::clear()
{
    for (auto& entry : m_channels)
    {
        entry.second->deactivateAll();
    }

    compactIfIdle();
}

EventChannelBase* EventRegistry::findChannel(EventTypeId eventType)
{
    const auto iterator = m_channels.find(eventType);
    return iterator != m_channels.end() ? iterator->second.get() : nullptr;
}

const EventChannelBase* EventRegistry::findChannel(EventTypeId eventType) const
{
    const auto iterator = m_channels.find(eventType);
    return iterator != m_channels.end() ? iterator->second.get() : nullptr;
}

void EventRegistry::compactIfIdle()
{
    if (m_dispatchDepth == 0)
    {
        compactAll();
    }
}

void EventRegistry::compactAll()
{
    for (auto& entry : m_channels)
    {
        entry.second->compact();
    }
}

}
