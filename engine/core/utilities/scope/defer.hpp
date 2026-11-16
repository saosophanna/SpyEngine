#pragma once

#include "core/utilities/scope/scope_guard.hpp"

#include <type_traits>
#include <utility>

namespace spy {

namespace detail {

struct DeferFactory
{
    template<typename Callback>
    [[nodiscard]] ScopeGuard<std::decay_t<Callback>> operator+(Callback&& callback) const
    {
        return makeScopeGuard(std::forward<Callback>(callback));
    }
};

}

}

#define SPY_UTILITIES_DETAIL_CONCAT_IMPL(lhs, rhs) lhs##rhs
#define SPY_UTILITIES_DETAIL_CONCAT(lhs, rhs) SPY_UTILITIES_DETAIL_CONCAT_IMPL(lhs, rhs)

#define SPY_DEFER                                                                                  \
    [[maybe_unused]] auto SPY_UTILITIES_DETAIL_CONCAT(spyDeferGuard_, __LINE__) =                  \
        ::spy::detail::DeferFactory{} + [&]()
