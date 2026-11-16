#include <cstddef>
#include <cstdint>
#include <limits>
#include <new>

#include "core/memory/allocation.hpp"

namespace spy::memory {

namespace {

struct BlockHeader
{
    void* base = nullptr;
    std::size_t size = 0;
};

}

bool isAligned(const void* pointer, std::size_t alignment) noexcept
{
    if (!isPowerOfTwo(alignment))
    {
        return false;
    }

    return reinterpret_cast<std::uintptr_t>(pointer) % alignment == 0;
}

void* allocateSystem(std::size_t size, std::size_t alignment) noexcept
{
    if (size == 0 || !isPowerOfTwo(alignment))
    {
        return nullptr;
    }

    const std::size_t effectiveAlignment =
        alignment < alignof(BlockHeader) ? alignof(BlockHeader) : alignment;

    if (size > std::numeric_limits<std::size_t>::max() - effectiveAlignment - sizeof(BlockHeader))
    {
        return nullptr;
    }

    const std::size_t total = size + effectiveAlignment + sizeof(BlockHeader);
    void* base = ::operator new(total, std::nothrow);

    if (base == nullptr)
    {
        return nullptr;
    }

    const std::uintptr_t unaligned =
        reinterpret_cast<std::uintptr_t>(base) + sizeof(BlockHeader);
    const std::uintptr_t aligned = unaligned + alignmentPadding(unaligned, effectiveAlignment);

    auto* header = reinterpret_cast<BlockHeader*>(aligned) - 1;
    header->base = base;
    header->size = size;

    return reinterpret_cast<void*>(aligned);
}

void deallocateSystem(void* pointer) noexcept
{
    if (pointer == nullptr)
    {
        return;
    }

    auto* header = static_cast<BlockHeader*>(pointer) - 1;
    ::operator delete(header->base);
}

}
