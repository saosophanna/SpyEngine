#pragma once

#include "core/utilities/types/types.hpp"

#include <cstddef>
#include <functional>
#include <limits>

namespace spy {

template<typename Tag>
class Handle
{
public:
    using Index = std::uint32_t;
    using Generation = std::uint32_t;

    static constexpr Index kInvalidIndex = std::numeric_limits<Index>::max();
    static constexpr Generation kInitialGeneration = 1;

    constexpr Handle() noexcept = default;

    constexpr Handle(Index index, Generation generation) noexcept
        : m_index(index), m_generation(generation)
    {
    }

    [[nodiscard]] constexpr Index index() const noexcept { return m_index; }
    [[nodiscard]] constexpr Generation generation() const noexcept { return m_generation; }

    [[nodiscard]] constexpr bool isValid() const noexcept
    {
        return m_index != kInvalidIndex && m_generation != 0;
    }

    constexpr explicit operator bool() const noexcept { return isValid(); }

    [[nodiscard]] constexpr std::uint64_t packed() const noexcept
    {
        return (static_cast<std::uint64_t>(m_generation) << 32) |
               static_cast<std::uint64_t>(m_index);
    }

    [[nodiscard]] constexpr std::uint64_t hashValue() const noexcept { return packed(); }

    friend constexpr bool operator==(Handle lhs, Handle rhs) noexcept = default;

private:
    Index m_index = kInvalidIndex;
    Generation m_generation = 0;
};

template<typename Tag>
struct HandleHash
{
    [[nodiscard]] std::size_t operator()(Handle<Tag> handle) const noexcept
    {
        return static_cast<std::size_t>(handle.packed());
    }
};

}

namespace std {

template<typename Tag>
struct hash<spy::Handle<Tag>>
{
    [[nodiscard]] size_t operator()(spy::Handle<Tag> handle) const noexcept
    {
        return static_cast<size_t>(handle.packed());
    }
};

}
