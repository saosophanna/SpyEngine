#pragma once

#include <cstddef>

#include "core/memory/allocator.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::memory {

class FreeListAllocator final : public IAllocator
{
public:
    explicit FreeListAllocator(usize capacity, IAllocator* backingAllocator = nullptr);
    ~FreeListAllocator() override;

    [[nodiscard]] void* allocate(
        usize size, usize alignment = kDefaultAlignment) noexcept override;

    void deallocate(void* pointer) noexcept override;
    void reset() noexcept override;

    [[nodiscard]] usize usedMemory() const noexcept override;
    [[nodiscard]] usize totalMemory() const noexcept override;
    [[nodiscard]] AllocatorStats stats() const noexcept override;
    [[nodiscard]] AllocatorCapabilities capabilities() const noexcept override;

    [[nodiscard]] usize capacity() const noexcept;
    [[nodiscard]] usize freeMemory() const noexcept;
    [[nodiscard]] usize largestFreeBlock() const noexcept;

private:
    struct BlockHeader;
    struct AllocationHeader;

    [[nodiscard]] void* allocateBacking(usize size, usize alignment) noexcept;
    void deallocateBacking(void* pointer) noexcept;
    void initializeBlock() noexcept;
    void removeFreeBlock(BlockHeader* block) noexcept;
    void insertFreeBlock(BlockHeader* block) noexcept;

    IAllocator* m_backingAllocator = nullptr;
    byte* m_memory = nullptr;
    BlockHeader* m_firstBlock = nullptr;
    BlockHeader* m_freeList = nullptr;
    usize m_capacity = 0;
    AllocatorStats m_stats;
};

}
