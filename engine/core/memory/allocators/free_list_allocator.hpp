#pragma once

#include <cstddef>

#include "core/memory/allocator.hpp"

namespace spy::memory {

class FreeListAllocator final : public IAllocator
{
public:
    explicit FreeListAllocator(std::size_t capacity, IAllocator* backingAllocator = nullptr);
    ~FreeListAllocator() override;

    FreeListAllocator(const FreeListAllocator&) = delete;
    FreeListAllocator& operator=(const FreeListAllocator&) = delete;

    [[nodiscard]] void* allocate(
        std::size_t size, std::size_t alignment = kDefaultAlignment) noexcept override;

    void deallocate(void* pointer) noexcept override;
    void reset() noexcept override;

    [[nodiscard]] std::size_t usedMemory() const noexcept override;
    [[nodiscard]] std::size_t totalMemory() const noexcept override;
    [[nodiscard]] AllocatorStats stats() const noexcept override;
    [[nodiscard]] AllocatorCapabilities capabilities() const noexcept override;

    [[nodiscard]] std::size_t capacity() const noexcept;
    [[nodiscard]] std::size_t freeMemory() const noexcept;
    [[nodiscard]] std::size_t largestFreeBlock() const noexcept;

private:
    struct BlockHeader;
    struct AllocationHeader;

    [[nodiscard]] void* allocateBacking(std::size_t size, std::size_t alignment) noexcept;
    void deallocateBacking(void* pointer) noexcept;
    void initializeBlock() noexcept;
    void removeFreeBlock(BlockHeader* block) noexcept;
    void insertFreeBlock(BlockHeader* block) noexcept;

    IAllocator* m_backingAllocator = nullptr;
    std::byte* m_memory = nullptr;
    BlockHeader* m_firstBlock = nullptr;
    BlockHeader* m_freeList = nullptr;
    std::size_t m_capacity = 0;
    AllocatorStats m_stats;
};

}
