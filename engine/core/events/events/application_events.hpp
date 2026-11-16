#pragma once

#include "core/events/event_traits.hpp"

namespace spy::events {

struct ApplicationStartedEvent
{
};

struct ApplicationQuitRequestedEvent
{
};

struct ApplicationStoppedEvent
{
};

struct ApplicationPausedEvent
{
};

struct ApplicationResumedEvent
{
};

struct ApplicationUpdatedEvent
{
    float64 deltaSeconds;
};

struct ApplicationFixedUpdatedEvent
{
    float64 deltaSeconds;
};

struct ApplicationRenderedEvent
{
    float64 deltaSeconds;
};

template<>
struct EventTraits<ApplicationStartedEvent>
{
    static constexpr EventCategory category = EventCategory::Application;
};

template<>
struct EventTraits<ApplicationQuitRequestedEvent>
{
    static constexpr EventCategory category = EventCategory::Application;
};

template<>
struct EventTraits<ApplicationStoppedEvent>
{
    static constexpr EventCategory category = EventCategory::Application;
};

template<>
struct EventTraits<ApplicationPausedEvent>
{
    static constexpr EventCategory category = EventCategory::Application;
};

template<>
struct EventTraits<ApplicationResumedEvent>
{
    static constexpr EventCategory category = EventCategory::Application;
};

template<>
struct EventTraits<ApplicationUpdatedEvent>
{
    static constexpr EventCategory category = EventCategory::Application;
};

template<>
struct EventTraits<ApplicationFixedUpdatedEvent>
{
    static constexpr EventCategory category = EventCategory::Application;
};

template<>
struct EventTraits<ApplicationRenderedEvent>
{
    static constexpr EventCategory category = EventCategory::Application;
};

}
