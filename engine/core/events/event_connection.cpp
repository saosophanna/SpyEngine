#include "core/events/event_connection.hpp"

#include "core/events/detail/event_registry.hpp"

#include <utility>

namespace spy::events {

EventConnection::EventConnection(std::weak_ptr<detail::EventRegistry> registry,
                                 EventTypeId eventType, EventConnectionId id) noexcept
    : m_registry(std::move(registry)), m_eventType(eventType), m_id(id)
{
}

void EventConnection::disconnect() noexcept
{
    if (auto registry = m_registry.lock())
    {
        registry->disconnect(m_eventType, m_id);
    }

    m_registry.reset();
    m_eventType = 0;
    m_id = EventConnectionId{};
}

bool EventConnection::isConnected() const
{
    const auto registry = m_registry.lock();
    return registry != nullptr && registry->isConnected(m_eventType, m_id);
}

}
