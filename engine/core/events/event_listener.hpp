#pragma once

#include "core/events/event_connection.hpp"
#include "core/events/event_subscription.hpp"

#include <cstddef>
#include <utility>

namespace spy::events {

class EventListener
{
public:
    EventListener() = default;
    EventListener(const EventListener&) = delete;
    EventListener& operator=(const EventListener&) = delete;
    EventListener(EventListener&&) noexcept = default;
    EventListener& operator=(EventListener&&) noexcept = default;
    ~EventListener() = default;

    void track(EventSubscription subscription)
    {
        m_subscriptions.add(std::move(subscription));
    }

    void track(EventConnection connection)
    {
        track(EventSubscription(std::move(connection)));
    }

    void disconnectAll() noexcept
    {
        m_subscriptions.disconnectAll();
    }

    [[nodiscard]] std::size_t subscriptionCount() const noexcept
    {
        return m_subscriptions.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return m_subscriptions.empty();
    }

protected:
    [[nodiscard]] EventSubscriptionGroup& subscriptions() noexcept
    {
        return m_subscriptions;
    }

    [[nodiscard]] const EventSubscriptionGroup& subscriptions() const noexcept
    {
        return m_subscriptions;
    }

private:
    EventSubscriptionGroup m_subscriptions;
};

}
