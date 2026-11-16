#pragma once

#include <cstddef>
#include <cstdint>
#include <new>

namespace spy::memory {

inline constexpr std::size_t kDefaultAlignment = alignof(std::max_align_t);

[[nodiscard]] constexpr bool isPowerOfTwo(std::size_t value) noexcept
{
    return value != 0 && (value & (value - 1)) == 0;
}

[[nodiscard]] constexpr std::size_t alignUp(std::size_t value, std::size_t alignment) noexcept
{
    const std::size_t remainder = value & (alignment - 1);
    return remainder == 0 ? value : value + (alignment - remainder);
}

[[nodiscard]] constexpr std::size_t alignmentPadding(
    std::uintptr_t address, std::size_t alignment) noexcept
{
    const std::size_t remainder = static_cast<std::size_t>(address & (alignment - 1));
    return remainder == 0 ? 0 : alignment - remainder;
}

[[nodiscard]] bool isAligned(const void* pointer, std::size_t alignment) noexcept;

[[nodiscard]] void* allocateSystem(std::size_t size, std::size_t alignment) noexcept;
void deallocateSystem(void* pointer) noexcept;

}
