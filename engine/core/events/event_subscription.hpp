#pragma once

#include "core/events/event_connection.hpp"

#include <cstddef>
#include <utility>
#include <vector>

namespace spy::events {

class EventSubscription
{
public:
    EventSubscription() noexcept = default;

    explicit EventSubscription(EventConnection connection) noexcept
        : m_connection(std::move(connection))
    {
    }

    EventSubscription(const EventSubscription&) = delete;
    EventSubscription& operator=(const EventSubscription&) = delete;

    EventSubscription(EventSubscription&& other) noexcept
        : m_connection(other.m_connection)
    {
        other.m_connection = EventConnection{};
    }

    EventSubscription& operator=(EventSubscription&& other) noexcept
    {
        if (this != &other)
        {
            disconnect();
            m_connection = other.m_connection;
            other.m_connection = EventConnection{};
        }

        return *this;
    }

    ~EventSubscription() noexcept
    {
        disconnect();
    }

    void disconnect() noexcept
    {
        m_connection.disconnect();
    }

    [[nodiscard]] bool isConnected() const
    {
        return m_connection.isConnected();
    }

    [[nodiscard]] const EventConnection& connection() const noexcept
    {
        return m_connection;
    }

    [[nodiscard]] EventConnection release() noexcept
    {
        EventConnection connection = m_connection;
        m_connection = EventConnection{};
        return connection;
    }

private:
    EventConnection m_connection;
};

class EventSubscriptionGroup
{
public:
    EventSubscriptionGroup() = default;
    EventSubscriptionGroup(const EventSubscriptionGroup&) = delete;
    EventSubscriptionGroup& operator=(const EventSubscriptionGroup&) = delete;
    EventSubscriptionGroup(EventSubscriptionGroup&&) noexcept = default;
    EventSubscriptionGroup& operator=(EventSubscriptionGroup&&) noexcept = default;
    ~EventSubscriptionGroup() = default;

    void add(EventSubscription subscription)
    {
        m_subscriptions.push_back(std::move(subscription));
    }

    void disconnectAll() noexcept
    {
        for (EventSubscription& subscription : m_subscriptions)
        {
            subscription.disconnect();
        }

        m_subscriptions.clear();
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return m_subscriptions.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return m_subscriptions.empty();
    }

    [[nodiscard]] std::size_t connectedCount() const
    {
        std::size_t count = 0;

        for (const EventSubscription& subscription : m_subscriptions)
        {
            if (subscription.isConnected())
            {
                ++count;
            }
        }

        return count;
    }

private:
    std::vector<EventSubscription> m_subscriptions;
};

}
