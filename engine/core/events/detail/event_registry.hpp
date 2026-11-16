#pragma once

#include "core/events/detail/event_channel.hpp"
#include "core/events/event_connection.hpp"
#include "core/events/event_type.hpp"
#include "core/utilities/scope/scope_guard.hpp"

#include <cstddef>
#include <memory>
#include <unordered_map>
#include <utility>

namespace spy::events::detail {

class EventRegistry final : public std::enable_shared_from_this<EventRegistry>
{
public:
    EventRegistry() = default;
    ~EventRegistry() = default;

    EventRegistry(const EventRegistry&) = delete;
    EventRegistry& operator=(const EventRegistry&) = delete;

    template<typename Event, typename Callback>
    EventConnection subscribe(Callback&& callback)
    {
        EventChannel<Event>& channel = channelFor<Event>();
        const EventConnectionId id = channel.add(std::forward<Callback>(callback));
        return EventConnection(weak_from_this(), eventTypeId<Event>(), id);
    }

    template<typename Event>
    [[nodiscard]] bool unsubscribe(EventConnectionId id)
    {
        EventChannelBase* channel = findChannel(eventTypeId<Event>());

        if (channel == nullptr)
        {
            return false;
        }

        const bool removed = channel->remove(id);
        compactIfIdle();
        return removed;
    }

    template<typename Event>
    void dispatch(const Event& event)
    {
        EventChannelBase* base = findChannel(eventTypeId<Event>());

        if (base == nullptr)
        {
            return;
        }

        ++m_dispatchDepth;
        auto depthGuard = makeScopeGuard([this]() noexcept {
            --m_dispatchDepth;
            compactIfIdle();
        });

        static_cast<EventChannel<Event>&>(*base).invoke(event);
    }

    template<typename Event>
    [[nodiscard]] bool hasSubscribers() const
    {
        const EventChannelBase* channel = findChannel(eventTypeId<Event>());
        return channel != nullptr && channel->size() > 0;
    }

    bool disconnect(EventTypeId eventType, EventConnectionId id)
    {
        EventChannelBase* channel = findChannel(eventType);

        if (channel == nullptr)
        {
            return false;
        }

        const bool removed = channel->remove(id);
        compactIfIdle();
        return removed;
    }

    [[nodiscard]] bool isConnected(EventTypeId eventType, EventConnectionId id) const
    {
        const EventChannelBase* channel = findChannel(eventType);
        return channel != nullptr && channel->contains(id);
    }

    [[nodiscard]] std::size_t subscriptionCount() const;
    [[nodiscard]] std::size_t eventTypeCount() const;
    [[nodiscard]] std::size_t dispatchDepth() const noexcept;
    void clear();

private:
    template<typename Event>
    EventChannel<Event>& channelFor()
    {
        const EventTypeId eventType = eventTypeId<Event>();
        auto iterator = m_channels.find(eventType);

        if (iterator != m_channels.end())
        {
            return static_cast<EventChannel<Event>&>(*iterator->second);
        }

        auto channel = std::make_unique<EventChannel<Event>>();
        EventChannel<Event>* raw = channel.get();
        m_channels.emplace(eventType, std::move(channel));
        return *raw;
    }

    [[nodiscard]] EventChannelBase* findChannel(EventTypeId eventType);
    [[nodiscard]] const EventChannelBase* findChannel(EventTypeId eventType) const;
    void compactIfIdle();
    void compactAll();

    std::unordered_map<EventTypeId, std::unique_ptr<EventChannelBase>> m_channels;
    std::size_t m_dispatchDepth = 0;
};

}
