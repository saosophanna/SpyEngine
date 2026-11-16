#pragma once

#include <cstdint>
#include <string_view>

namespace spy {

inline constexpr std::uint32_t kFnv1a32OffsetBasis = 2166136261u;
inline constexpr std::uint32_t kFnv1a32Prime = 16777619u;

inline constexpr std::uint64_t kFnv1a64OffsetBasis = 14695981039346656037ull;
inline constexpr std::uint64_t kFnv1a64Prime = 1099511628211ull;

constexpr std::uint32_t fnv1a32(std::string_view text) noexcept
{
    std::uint32_t hash = kFnv1a32OffsetBasis;

    for (char character : text)
    {
        hash ^= static_cast<std::uint8_t>(static_cast<unsigned char>(character));
        hash *= kFnv1a32Prime;
    }

    return hash;
}

constexpr std::uint64_t fnv1a64(std::string_view text) noexcept
{
    std::uint64_t hash = kFnv1a64OffsetBasis;

    for (char character : text)
    {
        hash ^= static_cast<std::uint8_t>(static_cast<unsigned char>(character));
        hash *= kFnv1a64Prime;
    }

    return hash;
}

}
