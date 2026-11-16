#pragma once

#include "core/events/event_connection.hpp"
#include "core/events/event_type.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

namespace spy::events::detail {

class EventChannelBase
{
public:
    EventChannelBase() = default;
    virtual ~EventChannelBase() = default;

    EventChannelBase(const EventChannelBase&) = delete;
    EventChannelBase& operator=(const EventChannelBase&) = delete;

    [[nodiscard]] virtual bool remove(EventConnectionId id) = 0;
    [[nodiscard]] virtual bool contains(EventConnectionId id) const = 0;
    [[nodiscard]] virtual std::size_t size() const = 0;
    virtual void deactivateAll() = 0;
    virtual void compact() = 0;
};

template<typename Event>
class EventChannel final : public EventChannelBase
{
    using Callback = std::function<void(const Event&)>;

    struct Handler
    {
        EventConnectionId id;
        Callback callback;
        bool active = true;
    };

public:
    EventConnectionId add(Callback callback)
    {
        const EventConnectionId id = m_ids.next();
        m_handlers.push_back(std::make_unique<Handler>(Handler{id, std::move(callback), true}));
        return id;
    }

    [[nodiscard]] bool remove(EventConnectionId id) override
    {
        for (const auto& handler : m_handlers)
        {
            if (handler->active && handler->id == id)
            {
                handler->active = false;
                m_dirty = true;
                return true;
            }
        }

        return false;
    }

    [[nodiscard]] bool contains(EventConnectionId id) const override
    {
        for (const auto& handler : m_handlers)
        {
            if (handler->active && handler->id == id)
            {
                return true;
            }
        }

        return false;
    }

    [[nodiscard]] std::size_t size() const override
    {
        std::size_t count = 0;

        for (const auto& handler : m_handlers)
        {
            if (handler->active)
            {
                ++count;
            }
        }

        return count;
    }

    void deactivateAll() override
    {
        for (const auto& handler : m_handlers)
        {
            handler->active = false;
        }

        m_dirty = true;
    }

    void compact() override
    {
        if (!m_dirty)
        {
            return;
        }

        m_handlers.erase(std::remove_if(m_handlers.begin(), m_handlers.end(),
                                        [](const std::unique_ptr<Handler>& handler) {
                                            return !handler->active;
                                        }),
                         m_handlers.end());
        m_dirty = false;
    }

    void invoke(const Event& event)
    {
        const std::size_t count = m_handlers.size();

        for (std::size_t index = 0; index < count; ++index)
        {
            Handler& handler = *m_handlers[index];

            if (handler.active)
            {
                handler.callback(event);
            }
        }
    }

private:
    std::vector<std::unique_ptr<Handler>> m_handlers;
    IdGenerator<EventConnectionTag> m_ids;
    bool m_dirty = false;
};

}
