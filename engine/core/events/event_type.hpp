#pragma once

#include "core/utilities/hashing/fnv1a.hpp"
#include "core/utilities/types/types.hpp"

#include <string_view>

namespace spy::events {

using EventTypeId = uint64;

template<typename Event>
constexpr EventTypeId eventTypeId() noexcept
{
#if defined(_MSC_VER)
    constexpr std::string_view signature = __FUNCSIG__;
#else
    constexpr std::string_view signature = __PRETTY_FUNCTION__;
#endif

    return fnv1a64(signature);
}

}
