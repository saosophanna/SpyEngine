#pragma once

#include <cstddef>

#include "core/memory/allocation.hpp"
#include "core/memory/statistics/allocator_stats.hpp"

namespace spy::memory {

struct AllocatorCapabilities
{
    bool individualDeallocation = false;
    bool reset = true;
    bool stackRewind = false;
    bool growth = false;
};

class IAllocator
{
public:
    virtual ~IAllocator() = default;

    IAllocator(const IAllocator&) = delete;
    IAllocator& operator=(const IAllocator&) = delete;

    [[nodiscard]] virtual void* allocate(
        std::size_t size, std::size_t alignment = kDefaultAlignment) noexcept = 0;

    virtual void deallocate(void* pointer) noexcept = 0;
    virtual void reset() noexcept = 0;

    [[nodiscard]] virtual std::size_t usedMemory() const noexcept = 0;
    [[nodiscard]] virtual std::size_t totalMemory() const noexcept = 0;
    [[nodiscard]] virtual AllocatorStats stats() const noexcept = 0;
    [[nodiscard]] virtual AllocatorCapabilities capabilities() const noexcept = 0;

protected:
    IAllocator() = default;
};

}
