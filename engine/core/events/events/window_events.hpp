#pragma once

#include "core/events/event_traits.hpp"

namespace spy::events {

struct WindowCreatedEvent
{
    uint32 width;
    uint32 height;
};

struct WindowCloseRequestedEvent
{
};

struct WindowClosedEvent
{
};

struct WindowResizedEvent
{
    uint32 width;
    uint32 height;
};

struct WindowMovedEvent
{
    int32 x;
    int32 y;
};

struct WindowFocusGainedEvent
{
};

struct WindowFocusLostEvent
{
};

struct WindowMinimizedEvent
{
};

struct WindowRestoredEvent
{
};

struct WindowDpiChangedEvent
{
    float32 scale;
    uint32 dpi;
};

template<>
struct EventTraits<WindowCreatedEvent>
{
    static constexpr EventCategory category = EventCategory::Window;
};

template<>
struct EventTraits<WindowCloseRequestedEvent>
{
    static constexpr EventCategory category = EventCategory::Window;
};

template<>
struct EventTraits<WindowClosedEvent>
{
    static constexpr EventCategory category = EventCategory::Window;
};

template<>
struct EventTraits<WindowResizedEvent>
{
    static constexpr EventCategory category = EventCategory::Window;
};

template<>
struct EventTraits<WindowMovedEvent>
{
    static constexpr EventCategory category = EventCategory::Window;
};

template<>
struct EventTraits<WindowFocusGainedEvent>
{
    static constexpr EventCategory category = EventCategory::Window;
};

template<>
struct EventTraits<WindowFocusLostEvent>
{
    static constexpr EventCategory category = EventCategory::Window;
};

template<>
struct EventTraits<WindowMinimizedEvent>
{
    static constexpr EventCategory category = EventCategory::Window;
};

template<>
struct EventTraits<WindowRestoredEvent>
{
    static constexpr EventCategory category = EventCategory::Window;
};

template<>
struct EventTraits<WindowDpiChangedEvent>
{
    static constexpr EventCategory category = EventCategory::Window;
};

}
