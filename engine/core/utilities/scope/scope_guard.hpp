#pragma once

#include <type_traits>
#include <utility>

namespace spy {

template<typename Callback>
class ScopeGuard
{
public:
    explicit ScopeGuard(Callback callback) noexcept(std::is_nothrow_move_constructible_v<Callback>)
        : m_callback(std::move(callback))
    {
    }

    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

    ScopeGuard(ScopeGuard&& other) noexcept(std::is_nothrow_move_constructible_v<Callback>)
        : m_callback(std::move(other.m_callback)), m_active(other.m_active)
    {
        other.m_active = false;
    }

    ScopeGuard& operator=(ScopeGuard&&) = delete;

    ~ScopeGuard() noexcept
    {
        if (m_active)
        {
            m_callback();
        }
    }

    void dismiss() noexcept { m_active = false; }

    [[nodiscard]] bool isActive() const noexcept { return m_active; }

private:
    Callback m_callback;
    bool m_active = true;
};

template<typename Callback>
[[nodiscard]] ScopeGuard<std::decay_t<Callback>> makeScopeGuard(Callback&& callback) noexcept(
    std::is_nothrow_constructible_v<std::decay_t<Callback>, Callback&&>)
{
    return ScopeGuard<std::decay_t<Callback>>(std::forward<Callback>(callback));
}

}
