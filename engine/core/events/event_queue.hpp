#pragma once

#include "core/events/event_connection.hpp"
#include "core/events/event_dispatcher.hpp"
#include "core/events/event_type.hpp"

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace spy::events {

namespace detail {

class QueuedEvent
{
public:
    QueuedEvent() = default;
    virtual ~QueuedEvent() = default;

    QueuedEvent(const QueuedEvent&) = delete;
    QueuedEvent& operator=(const QueuedEvent&) = delete;

    [[nodiscard]] virtual EventTypeId eventType() const noexcept = 0;
    virtual void dispatch(EventDispatcher& dispatcher) const = 0;
};

template<typename Event>
class TypedQueuedEvent final : public QueuedEvent
{
public:
    template<typename... Args>
    explicit TypedQueuedEvent(Args&&... args)
        : m_event(std::forward<Args>(args)...)
    {
    }

    [[nodiscard]] EventTypeId eventType() const noexcept override
    {
        return eventTypeId<Event>();
    }

    void dispatch(EventDispatcher& dispatcher) const override
    {
        dispatcher.dispatch(m_event);
    }

private:
    Event m_event;
};

}

class EventQueue
{
public:
    EventQueue() = default;
    EventQueue(const EventQueue&) = delete;
    EventQueue& operator=(const EventQueue&) = delete;
    EventQueue(EventQueue&&) noexcept = default;
    EventQueue& operator=(EventQueue&&) noexcept = default;
    ~EventQueue() = default;

    template<typename Event>
    void enqueue(Event event)
    {
        m_pending.push_back(
            std::make_unique<detail::TypedQueuedEvent<Event>>(std::move(event)));
    }

    template<typename Event, typename... Args>
    void emplace(Args&&... args)
    {
        static_assert(std::is_constructible_v<Event, Args...>);
        m_pending.push_back(
            std::make_unique<detail::TypedQueuedEvent<Event>>(std::forward<Args>(args)...));
    }

    void process(EventDispatcher& dispatcher);
    void clear();

    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool processing() const noexcept;

    template<typename Event>
    [[nodiscard]] bool contains() const
    {
        const EventTypeId type = eventTypeId<Event>();

        for (const auto& event : m_pending)
        {
            if (event->eventType() == type)
            {
                return true;
            }
        }

        for (const auto& event : m_processing)
        {
            if (event->eventType() == type)
            {
                return true;
            }
        }

        return false;
    }

private:
    std::vector<std::unique_ptr<detail::QueuedEvent>> m_pending;
    std::vector<std::unique_ptr<detail::QueuedEvent>> m_processing;
    bool m_processingActive = false;
};

}
