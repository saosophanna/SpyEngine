#pragma once

#include "core/events/event_traits.hpp"

namespace spy::events {

enum class KeyCode : uint16
{
    Unknown = 0,
    Space,
    Apostrophe,
    Comma,
    Minus,
    Period,
    Slash,
    Semicolon,
    Equal,
    LeftBracket,
    Backslash,
    RightBracket,
    GraveAccent,
    Digit0,
    Digit1,
    Digit2,
    Digit3,
    Digit4,
    Digit5,
    Digit6,
    Digit7,
    Digit8,
    Digit9,
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    Escape,
    Enter,
    Tab,
    Backspace,
    Insert,
    Delete,
    Right,
    Left,
    Down,
    Up,
    PageUp,
    PageDown,
    Home,
    End,
    CapsLock,
    ScrollLock,
    NumLock,
    PrintScreen,
    Pause,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    Keypad0,
    Keypad1,
    Keypad2,
    Keypad3,
    Keypad4,
    Keypad5,
    Keypad6,
    Keypad7,
    Keypad8,
    Keypad9,
    KeypadDecimal,
    KeypadDivide,
    KeypadMultiply,
    KeypadSubtract,
    KeypadAdd,
    KeypadEnter,
    KeypadEqual,
    LeftShift,
    LeftControl,
    LeftAlt,
    LeftSuper,
    RightShift,
    RightControl,
    RightAlt,
    RightSuper,
    Menu
};

enum class KeyModifier : uint8
{
    None = 0,
    Shift = 1u << 0u,
    Control = 1u << 1u,
    Alt = 1u << 2u,
    Super = 1u << 3u,
    CapsLock = 1u << 4u,
    NumLock = 1u << 5u
};

constexpr KeyModifier operator|(KeyModifier lhs, KeyModifier rhs) noexcept
{
    return static_cast<KeyModifier>(static_cast<uint8>(lhs) | static_cast<uint8>(rhs));
}

constexpr KeyModifier operator&(KeyModifier lhs, KeyModifier rhs) noexcept
{
    return static_cast<KeyModifier>(static_cast<uint8>(lhs) & static_cast<uint8>(rhs));
}

constexpr KeyModifier& operator|=(KeyModifier& lhs, KeyModifier rhs) noexcept
{
    lhs = lhs | rhs;
    return lhs;
}

constexpr KeyModifier& operator&=(KeyModifier& lhs, KeyModifier rhs) noexcept
{
    lhs = lhs & rhs;
    return lhs;
}

[[nodiscard]] constexpr bool hasModifier(KeyModifier value, KeyModifier modifier) noexcept
{
    return (value & modifier) != KeyModifier::None;
}

enum class MouseButton : uint8
{
    Unknown = 0,
    Left,
    Right,
    Middle,
    Button4,
    Button5,
    Button6,
    Button7,
    Button8
};

struct KeyPressedEvent
{
    KeyCode key;
    KeyModifier modifiers;
    bool repeat;
};

struct KeyReleasedEvent
{
    KeyCode key;
    KeyModifier modifiers;
};

struct TextInputEvent
{
    uint32 codepoint;
};

struct MouseMovedEvent
{
    float32 x;
    float32 y;
    float32 deltaX;
    float32 deltaY;
};

struct MouseScrolledEvent
{
    float32 offsetX;
    float32 offsetY;
};

struct MouseButtonPressedEvent
{
    MouseButton button;
    KeyModifier modifiers;
    float32 x;
    float32 y;
};

struct MouseButtonReleasedEvent
{
    MouseButton button;
    KeyModifier modifiers;
    float32 x;
    float32 y;
};

struct MouseEnteredEvent
{
};

struct MouseLeftEvent
{
};

template<>
struct EventTraits<KeyPressedEvent>
{
    static constexpr EventCategory category = EventCategory::Input;
};

template<>
struct EventTraits<KeyReleasedEvent>
{
    static constexpr EventCategory category = EventCategory::Input;
};

template<>
struct EventTraits<TextInputEvent>
{
    static constexpr EventCategory category = EventCategory::Input;
};

template<>
struct EventTraits<MouseMovedEvent>
{
    static constexpr EventCategory category = EventCategory::Input;
};

template<>
struct EventTraits<MouseScrolledEvent>
{
    static constexpr EventCategory category = EventCategory::Input;
};

template<>
struct EventTraits<MouseButtonPressedEvent>
{
    static constexpr EventCategory category = EventCategory::Input;
};

template<>
struct EventTraits<MouseButtonReleasedEvent>
{
    static constexpr EventCategory category = EventCategory::Input;
};

template<>
struct EventTraits<MouseEnteredEvent>
{
    static constexpr EventCategory category = EventCategory::Input;
};

template<>
struct EventTraits<MouseLeftEvent>
{
    static constexpr EventCategory category = EventCategory::Input;
};

}
