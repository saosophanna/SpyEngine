#pragma once

#include <cstddef>

#include "core/memory/allocator.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::memory {

class PoolAllocator final : public IAllocator
{
public:
    PoolAllocator(
        usize objectSize,
        usize objectAlignment,
        usize capacity,
        IAllocator* backingAllocator = nullptr);

    ~PoolAllocator() override;

    using IAllocator::allocate;

    [[nodiscard]] void* allocate(
        usize size, usize alignment = kDefaultAlignment) noexcept override;

    [[nodiscard]] void* allocate() noexcept;

    void deallocate(void* pointer) noexcept override;
    void reset() noexcept override;

    [[nodiscard]] usize usedMemory() const noexcept override;
    [[nodiscard]] usize totalMemory() const noexcept override;
    [[nodiscard]] AllocatorStats stats() const noexcept override;
    [[nodiscard]] AllocatorCapabilities capabilities() const noexcept override;

    [[nodiscard]] usize objectSize() const noexcept;
    [[nodiscard]] usize objectAlignment() const noexcept;
    [[nodiscard]] usize stride() const noexcept;
    [[nodiscard]] usize capacity() const noexcept;
    [[nodiscard]] usize freeCount() const noexcept;

private:
    [[nodiscard]] void* allocateBacking(usize size, usize alignment) noexcept;
    void deallocateBacking(void* pointer) noexcept;
    [[nodiscard]] bool isFreeBlock(const void* block) const noexcept;
    void rebuildFreeList() noexcept;
    void updateUsedMemory() noexcept;

    IAllocator* m_backingAllocator = nullptr;
    byte* m_memory = nullptr;
    void* m_freeList = nullptr;
    usize m_objectSize = 0;
    usize m_objectAlignment = 0;
    usize m_stride = 0;
    usize m_capacity = 0;
    usize m_freeCount = 0;
    AllocatorStats m_stats;
};

}
