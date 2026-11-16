#pragma once

#include "core/events/event_type.hpp"
#include "core/utilities/types/id.hpp"

#include <memory>

namespace spy::events {

struct EventConnectionTag;

using EventConnectionId = Id<EventConnectionTag>;

namespace detail {

class EventRegistry;

}

class EventConnection
{
public:
    EventConnection() noexcept = default;
    EventConnection(const EventConnection&) noexcept = default;
    EventConnection& operator=(const EventConnection&) noexcept = default;
    ~EventConnection() = default;

    void disconnect() noexcept;

    [[nodiscard]] bool isConnected() const;

    [[nodiscard]] explicit operator bool() const
    {
        return isConnected();
    }

    [[nodiscard]] EventConnectionId id() const noexcept
    {
        return m_id;
    }

    [[nodiscard]] EventTypeId eventType() const noexcept
    {
        return m_eventType;
    }

private:
    friend class detail::EventRegistry;

    EventConnection(std::weak_ptr<detail::EventRegistry> registry, EventTypeId eventType,
                    EventConnectionId id) noexcept;

    std::weak_ptr<detail::EventRegistry> m_registry;
    EventTypeId m_eventType = 0;
    EventConnectionId m_id;
};

}
