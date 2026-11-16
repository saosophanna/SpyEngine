#pragma once

#include <cstddef>
#include <cstdint>

#include "core/memory/allocator.hpp"

namespace spy::memory {

class ArenaAllocator final : public IAllocator
{
public:
    static constexpr std::size_t kDefaultBlockSize = 4096;

    explicit ArenaAllocator(std::size_t blockSize, IAllocator* backingAllocator = nullptr);
    ~ArenaAllocator() override;

    ArenaAllocator(const ArenaAllocator&) = delete;
    ArenaAllocator& operator=(const ArenaAllocator&) = delete;

    [[nodiscard]] void* allocate(
        std::size_t size, std::size_t alignment = kDefaultAlignment) noexcept override;

    void deallocate(void* pointer) noexcept override;
    void reset() noexcept override;

    [[nodiscard]] std::size_t usedMemory() const noexcept override;
    [[nodiscard]] std::size_t totalMemory() const noexcept override;
    [[nodiscard]] AllocatorStats stats() const noexcept override;
    [[nodiscard]] AllocatorCapabilities capabilities() const noexcept override;

    [[nodiscard]] std::size_t blockSize() const noexcept;
    [[nodiscard]] std::size_t blockCount() const noexcept;

    void release() noexcept;

private:
    struct Block
    {
        Block* next = nullptr;
        std::byte* begin = nullptr;
        std::byte* current = nullptr;
        std::byte* end = nullptr;
    };

    [[nodiscard]] void* allocateBacking(std::size_t size, std::size_t alignment) noexcept;
    void deallocateBacking(void* pointer) noexcept;
    [[nodiscard]] Block* createBlock(std::size_t payloadSize) noexcept;
    void recordAllocation(std::size_t bytes) noexcept;

    IAllocator* m_backingAllocator = nullptr;
    Block* m_blocks = nullptr;
    Block* m_current = nullptr;
    std::size_t m_blockSize = 0;
    std::size_t m_blockCount = 0;
    AllocatorStats m_stats;
};

}
