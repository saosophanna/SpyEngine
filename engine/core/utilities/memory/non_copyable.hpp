#pragma once

namespace spy {

class NonCopyable
{
protected:
    constexpr NonCopyable() noexcept = default;
    constexpr NonCopyable(NonCopyable&&) noexcept = default;
    constexpr NonCopyable& operator=(NonCopyable&&) noexcept = default;
    ~NonCopyable() = default;

    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};

}
