#pragma once

#include "core/events/event.hpp"

namespace spy::events {

template<typename Event>
struct EventTraits
{
    static constexpr EventCategory category = EventCategory::None;
};

template<typename Event>
inline constexpr EventCategory eventCategory = EventTraits<Event>::category;

}
