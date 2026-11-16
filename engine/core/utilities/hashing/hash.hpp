#pragma once

#include "core/utilities/hashing/fnv1a.hpp"

#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <type_traits>

namespace spy {

inline constexpr std::uint64_t kHashCombineConstant = 0x9e3779b97f4a7c15ull;

constexpr std::uint64_t hashCombine(std::uint64_t seed, std::uint64_t value) noexcept
{
    std::uint64_t mixed = seed ^ (value + kHashCombineConstant + (seed << 6) + (seed >> 2));
    mixed ^= mixed >> 33;
    mixed *= 0xff51afd7ed558ccdull;
    mixed ^= mixed >> 33;
    mixed *= 0xc4ceb9fe1a85ec53ull;
    mixed ^= mixed >> 33;
    return mixed;
}

inline std::uint64_t hashBytes(std::span<const std::byte> bytes) noexcept
{
    std::uint64_t hash = kFnv1a64OffsetBasis;

    for (std::byte value : bytes)
    {
        hash ^= static_cast<std::uint8_t>(value);
        hash *= kFnv1a64Prime;
    }

    return hash;
}

constexpr std::uint64_t hashValue(std::string_view text) noexcept
{
    return fnv1a64(text);
}

template<std::size_t Count>
constexpr std::uint64_t hashValue(const char (&text)[Count]) noexcept
{
    return fnv1a64(std::string_view(text, Count - 1));
}

template<typename T>
    requires std::integral<T> || std::is_enum_v<T>
constexpr std::uint64_t hashValue(T value) noexcept
{
    return static_cast<std::uint64_t>(value);
}

template<typename T>
    requires(std::floating_point<T> && (sizeof(T) == sizeof(std::uint32_t) ||
                                        sizeof(T) == sizeof(std::uint64_t)))
constexpr std::uint64_t hashValue(T value) noexcept
{
    if constexpr (sizeof(T) == sizeof(std::uint32_t))
    {
        return hashCombine(0x8da6b343ull, std::bit_cast<std::uint32_t>(value));
    }
    else
    {
        return hashCombine(0xd8163841ull, std::bit_cast<std::uint64_t>(value));
    }
}

constexpr std::uint64_t hashValue(const char* text) noexcept
{
    return fnv1a64(std::string_view(text));
}

template<typename T>
    requires(std::is_pointer_v<T> &&
             !std::same_as<std::remove_cv_t<std::remove_pointer_t<T>>, char>)
constexpr std::uint64_t hashValue(T value) noexcept
{
    return static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(value));
}

template<typename T>
    requires requires(const T& value) {
        { value.hashValue() } -> std::convertible_to<std::uint64_t>;
    }
constexpr std::uint64_t hashValue(const T& value) noexcept
{
    return static_cast<std::uint64_t>(value.hashValue());
}

template<typename T>
    requires(std::has_unique_object_representations_v<T> && !std::is_arithmetic_v<T> &&
             !std::is_enum_v<T> && !std::is_pointer_v<T> && !std::is_array_v<T> &&
             !std::same_as<T, std::string_view> && !requires(const T& value) {
                 value.hashValue();
             })
std::uint64_t hashValue(const T& value) noexcept
{
    return hashBytes(std::as_bytes(std::span<const T>(&value, 1)));
}

template<typename T>
struct Hash
{
    [[nodiscard]] std::size_t operator()(const T& value) const noexcept
    {
        return static_cast<std::size_t>(hashValue(value));
    }
};

}
