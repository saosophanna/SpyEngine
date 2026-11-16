#pragma once

#include <cstdint>

namespace spy::platform
{

enum class Key : std::uint32_t
{
    Unknown = 0,
    Escape,
    Enter,
    Space,
    Left,
    Right,
    Up,
    Down,
    Home,
    End,
    PageUp,
    PageDown,
    A,
    D,
    R,
    S,
    W,
};

constexpr std::uint32_t kKeyCount = static_cast<std::uint32_t>(Key::W) + 1;

}
