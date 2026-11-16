#pragma once

#include <cstddef>

#include "core/memory/allocation.hpp"
#include "core/memory/statistics/allocator_stats.hpp"
#include "core/utilities/memory/non_copyable.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::memory {

struct AllocatorCapabilities
{
    bool individualDeallocation = false;
    bool reset = true;
    bool stackRewind = false;
    bool growth = false;
};

class IAllocator : public NonCopyable
{
public:
    virtual ~IAllocator() = default;

    [[nodiscard]] virtual void* allocate(
        usize size, usize alignment = kDefaultAlignment) noexcept = 0;

    virtual void deallocate(void* pointer) noexcept = 0;
    virtual void reset() noexcept = 0;

    [[nodiscard]] virtual usize usedMemory() const noexcept = 0;
    [[nodiscard]] virtual usize totalMemory() const noexcept = 0;
    [[nodiscard]] virtual AllocatorStats stats() const noexcept = 0;
    [[nodiscard]] virtual AllocatorCapabilities capabilities() const noexcept = 0;

protected:
    IAllocator() = default;
};

}
