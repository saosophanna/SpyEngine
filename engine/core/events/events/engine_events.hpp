#pragma once

#include "core/events/event_traits.hpp"

namespace spy::events {

struct EngineInitializedEvent
{
};

struct EngineShutdownEvent
{
};

struct FrameBeginEvent
{
    uint64 frameIndex;
    float64 deltaSeconds;
};

struct FrameEndEvent
{
    uint64 frameIndex;
    float64 deltaSeconds;
};

struct FrameRenderEvent
{
    uint64 frameIndex;
    float64 deltaSeconds;
};

struct FrameFixedUpdateEvent
{
    uint64 tickIndex;
    float64 deltaSeconds;
};

template<>
struct EventTraits<EngineInitializedEvent>
{
    static constexpr EventCategory category = EventCategory::Engine;
};

template<>
struct EventTraits<EngineShutdownEvent>
{
    static constexpr EventCategory category = EventCategory::Engine;
};

template<>
struct EventTraits<FrameBeginEvent>
{
    static constexpr EventCategory category = EventCategory::Engine;
};

template<>
struct EventTraits<FrameEndEvent>
{
    static constexpr EventCategory category = EventCategory::Engine;
};

template<>
struct EventTraits<FrameRenderEvent>
{
    static constexpr EventCategory category = EventCategory::Engine;
};

template<>
struct EventTraits<FrameFixedUpdateEvent>
{
    static constexpr EventCategory category = EventCategory::Engine;
};

}
