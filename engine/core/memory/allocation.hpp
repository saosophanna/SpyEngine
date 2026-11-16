#pragma once

#include <cstddef>
#include <cstdint>
#include <new>

#include "core/utilities/types/types.hpp"

namespace spy::memory {

inline constexpr usize kDefaultAlignment = alignof(std::max_align_t);

[[nodiscard]] constexpr bool isPowerOfTwo(usize value) noexcept
{
    return value != 0 && (value & (value - 1)) == 0;
}

[[nodiscard]] constexpr usize alignUp(usize value, usize alignment) noexcept
{
    const usize remainder = value & (alignment - 1);
    return remainder == 0 ? value : value + (alignment - remainder);
}

[[nodiscard]] constexpr usize alignmentPadding(
    std::uintptr_t address, usize alignment) noexcept
{
    const usize remainder = static_cast<usize>(address & (alignment - 1));
    return remainder == 0 ? 0 : alignment - remainder;
}

[[nodiscard]] bool isAligned(const void* pointer, usize alignment) noexcept;

[[nodiscard]] void* allocateSystem(usize size, usize alignment) noexcept;
void deallocateSystem(void* pointer) noexcept;

}
