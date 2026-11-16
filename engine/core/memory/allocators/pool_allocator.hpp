#pragma once

#include <cstddef>

#include "core/memory/allocator.hpp"

namespace spy::memory {

class PoolAllocator final : public IAllocator
{
public:
    PoolAllocator(
        std::size_t objectSize,
        std::size_t objectAlignment,
        std::size_t capacity,
        IAllocator* backingAllocator = nullptr);

    ~PoolAllocator() override;

    PoolAllocator(const PoolAllocator&) = delete;
    PoolAllocator& operator=(const PoolAllocator&) = delete;

    using IAllocator::allocate;

    [[nodiscard]] void* allocate(
        std::size_t size, std::size_t alignment = kDefaultAlignment) noexcept override;

    [[nodiscard]] void* allocate() noexcept;

    void deallocate(void* pointer) noexcept override;
    void reset() noexcept override;

    [[nodiscard]] std::size_t usedMemory() const noexcept override;
    [[nodiscard]] std::size_t totalMemory() const noexcept override;
    [[nodiscard]] AllocatorStats stats() const noexcept override;
    [[nodiscard]] AllocatorCapabilities capabilities() const noexcept override;

    [[nodiscard]] std::size_t objectSize() const noexcept;
    [[nodiscard]] std::size_t objectAlignment() const noexcept;
    [[nodiscard]] std::size_t stride() const noexcept;
    [[nodiscard]] std::size_t capacity() const noexcept;
    [[nodiscard]] std::size_t freeCount() const noexcept;

private:
    [[nodiscard]] void* allocateBacking(std::size_t size, std::size_t alignment) noexcept;
    void deallocateBacking(void* pointer) noexcept;
    [[nodiscard]] bool isFreeBlock(const void* block) const noexcept;
    void rebuildFreeList() noexcept;
    void updateUsedMemory() noexcept;

    IAllocator* m_backingAllocator = nullptr;
    std::byte* m_memory = nullptr;
    void* m_freeList = nullptr;
    std::size_t m_objectSize = 0;
    std::size_t m_objectAlignment = 0;
    std::size_t m_stride = 0;
    std::size_t m_capacity = 0;
    std::size_t m_freeCount = 0;
    AllocatorStats m_stats;
};

}
