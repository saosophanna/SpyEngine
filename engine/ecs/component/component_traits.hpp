#pragma once

#include <type_traits>

#include "ecs/component/component.hpp"

namespace spy::ecs {

template<Component T>
struct ComponentTraits
{
    using Type = T;

    static constexpr bool kIsTriviallyCopyable = std::is_trivially_copyable_v<T>;
    static constexpr bool kIsTriviallyDestructible = std::is_trivially_destructible_v<T>;
    static constexpr bool kIsEmpty = std::is_empty_v<T>;
};

}
