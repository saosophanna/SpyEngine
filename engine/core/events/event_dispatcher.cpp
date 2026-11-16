#include "core/events/event_dispatcher.hpp"

#include <memory>

namespace spy::events {

EventDispatcher::EventDispatcher()
    : m_registry(std::make_shared<detail::EventRegistry>())
{
}

bool EventDispatcher::isConnected(EventTypeId eventType, EventConnectionId id) const
{
    return m_registry->isConnected(eventType, id);
}

bool EventDispatcher::disconnect(EventTypeId eventType, EventConnectionId id)
{
    return m_registry->disconnect(eventType, id);
}

std::size_t EventDispatcher::subscriptionCount() const
{
    return m_registry->subscriptionCount();
}

std::size_t EventDispatcher::eventTypeCount() const
{
    return m_registry->eventTypeCount();
}

std::size_t EventDispatcher::dispatchDepth() const noexcept
{
    return m_registry->dispatchDepth();
}

void EventDispatcher::clear()
{
    m_registry->clear();
}

}
