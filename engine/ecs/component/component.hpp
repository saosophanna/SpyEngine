#pragma once

#include <type_traits>

namespace spy::ecs {

template<typename T>
concept Component =
    std::is_object_v<T> &&
    !std::is_array_v<T> &&
    !std::is_const_v<T> &&
    !std::is_volatile_v<T> &&
    std::is_move_constructible_v<T> &&
    std::is_move_assignable_v<T> &&
    std::is_destructible_v<T>;

template<typename T>
concept QueryComponent =
    Component<std::remove_const_t<T>> &&
    !std::is_volatile_v<T>;

}
