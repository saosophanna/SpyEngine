#pragma once

#include "core/utilities/types/types.hpp"

namespace spy::events {

enum class EventCategory : uint32
{
    None = 0,
    Application = 1u << 0u,
    Window = 1u << 1u,
    Input = 1u << 2u,
    Engine = 1u << 3u
};

constexpr EventCategory operator|(EventCategory lhs, EventCategory rhs) noexcept
{
    return static_cast<EventCategory>(static_cast<uint32>(lhs) | static_cast<uint32>(rhs));
}

constexpr EventCategory operator&(EventCategory lhs, EventCategory rhs) noexcept
{
    return static_cast<EventCategory>(static_cast<uint32>(lhs) & static_cast<uint32>(rhs));
}

constexpr EventCategory& operator|=(EventCategory& lhs, EventCategory rhs) noexcept
{
    lhs = lhs | rhs;
    return lhs;
}

constexpr EventCategory& operator&=(EventCategory& lhs, EventCategory rhs) noexcept
{
    lhs = lhs & rhs;
    return lhs;
}

[[nodiscard]] constexpr bool hasCategory(EventCategory value, EventCategory category) noexcept
{
    return (value & category) != EventCategory::None;
}

}
