#pragma once

#include "core/events/event_connection.hpp"
#include "core/events/event_dispatcher.hpp"
#include "core/events/event_queue.hpp"

#include <concepts>
#include <cstddef>
#include <utility>

namespace spy::events {

class EventBus
{
public:
    EventBus() = default;
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;
    EventBus(EventBus&&) = delete;
    EventBus& operator=(EventBus&&) = delete;
    ~EventBus() = default;

    template<typename Event, typename Callback>
        requires std::invocable<Callback&, const Event&>
    EventConnection subscribe(Callback&& callback)
    {
        return m_dispatcher.subscribe<Event>(std::forward<Callback>(callback));
    }

    template<typename Event>
    [[nodiscard]] bool unsubscribe(EventConnectionId id)
    {
        return m_dispatcher.unsubscribe<Event>(id);
    }

    template<typename Event>
    void publish(const Event& event)
    {
        m_dispatcher.dispatch<Event>(event);
    }

    template<typename Event>
    void enqueue(Event event)
    {
        m_queue.enqueue(std::move(event));
    }

    template<typename Event, typename... Args>
    void emplace(Args&&... args)
    {
        m_queue.emplace<Event>(std::forward<Args>(args)...);
    }

    void processQueuedEvents()
    {
        m_queue.process(m_dispatcher);
    }

    void clearQueue()
    {
        m_queue.clear();
    }

    template<typename Event>
    [[nodiscard]] bool hasSubscribers() const
    {
        return m_dispatcher.hasSubscribers<Event>();
    }

    [[nodiscard]] std::size_t subscriptionCount() const
    {
        return m_dispatcher.subscriptionCount();
    }

    [[nodiscard]] std::size_t queuedEventCount() const
    {
        return m_queue.size();
    }

    [[nodiscard]] bool hasQueuedEvents() const
    {
        return !m_queue.empty();
    }

    [[nodiscard]] EventDispatcher& dispatcher() noexcept
    {
        return m_dispatcher;
    }

    [[nodiscard]] const EventDispatcher& dispatcher() const noexcept
    {
        return m_dispatcher;
    }

    [[nodiscard]] EventQueue& queue() noexcept
    {
        return m_queue;
    }

    [[nodiscard]] const EventQueue& queue() const noexcept
    {
        return m_queue;
    }

    void clear();

private:
    EventDispatcher m_dispatcher;
    EventQueue m_queue;
};

}
