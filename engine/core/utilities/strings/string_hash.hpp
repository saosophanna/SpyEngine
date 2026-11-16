#pragma once

#include "core/utilities/hashing/fnv1a.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string_view>

namespace spy {

class StringHash
{
public:
    constexpr StringHash() noexcept = default;
    constexpr explicit StringHash(std::string_view text) noexcept : m_value(fnv1a64(text)) {}

    [[nodiscard]] constexpr std::uint64_t value() const noexcept { return m_value; }
    [[nodiscard]] constexpr bool isValid() const noexcept { return m_value != 0; }

    friend constexpr bool operator==(StringHash lhs, StringHash rhs) noexcept = default;

private:
    std::uint64_t m_value = 0;
};

struct StringHashHasher
{
    [[nodiscard]] std::size_t operator()(std::string_view text) const noexcept
    {
        return static_cast<std::size_t>(fnv1a64(text));
    }
};

}

namespace std {

template<>
struct hash<spy::StringHash>
{
    [[nodiscard]] size_t operator()(spy::StringHash hash) const noexcept
    {
        return static_cast<size_t>(hash.value());
    }
};

}
