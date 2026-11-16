#pragma once

#include "core/events/detail/event_registry.hpp"
#include "core/events/event_connection.hpp"
#include "core/events/event_type.hpp"

#include <concepts>
#include <cstddef>
#include <memory>
#include <utility>

namespace spy::events {

class EventDispatcher
{
public:
    EventDispatcher();
    EventDispatcher(const EventDispatcher&) = delete;
    EventDispatcher& operator=(const EventDispatcher&) = delete;
    EventDispatcher(EventDispatcher&&) = delete;
    EventDispatcher& operator=(EventDispatcher&&) = delete;
    ~EventDispatcher() = default;

    template<typename Event, typename Callback>
        requires std::invocable<Callback&, const Event&>
    EventConnection subscribe(Callback&& callback)
    {
        return m_registry->subscribe<Event>(std::forward<Callback>(callback));
    }

    template<typename Event>
    [[nodiscard]] bool unsubscribe(EventConnectionId id)
    {
        return m_registry->unsubscribe<Event>(id);
    }

    template<typename Event>
    void dispatch(const Event& event)
    {
        m_registry->dispatch<Event>(event);
    }

    template<typename Event>
    [[nodiscard]] bool hasSubscribers() const
    {
        return m_registry->hasSubscribers<Event>();
    }

    [[nodiscard]] bool isConnected(EventTypeId eventType, EventConnectionId id) const;
    bool disconnect(EventTypeId eventType, EventConnectionId id);
    [[nodiscard]] std::size_t subscriptionCount() const;
    [[nodiscard]] std::size_t eventTypeCount() const;
    [[nodiscard]] std::size_t dispatchDepth() const noexcept;
    void clear();

private:
    std::shared_ptr<detail::EventRegistry> m_registry;
};

}
