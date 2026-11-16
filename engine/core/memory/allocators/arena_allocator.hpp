#pragma once

#include <cstddef>
#include <cstdint>

#include "core/memory/allocator.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::memory {

class ArenaAllocator final : public IAllocator
{
public:
    static constexpr usize kDefaultBlockSize = 4096;

    explicit ArenaAllocator(usize blockSize, IAllocator* backingAllocator = nullptr);
    ~ArenaAllocator() override;

    [[nodiscard]] void* allocate(
        usize size, usize alignment = kDefaultAlignment) noexcept override;

    void deallocate(void* pointer) noexcept override;
    void reset() noexcept override;

    [[nodiscard]] usize usedMemory() const noexcept override;
    [[nodiscard]] usize totalMemory() const noexcept override;
    [[nodiscard]] AllocatorStats stats() const noexcept override;
    [[nodiscard]] AllocatorCapabilities capabilities() const noexcept override;

    [[nodiscard]] usize blockSize() const noexcept;
    [[nodiscard]] usize blockCount() const noexcept;

    void release() noexcept;

private:
    struct Block
    {
        Block* next = nullptr;
        byte* begin = nullptr;
        byte* current = nullptr;
        byte* end = nullptr;
    };

    [[nodiscard]] void* allocateBacking(usize size, usize alignment) noexcept;
    void deallocateBacking(void* pointer) noexcept;
    [[nodiscard]] Block* createBlock(usize payloadSize) noexcept;
    void recordAllocation(usize bytes) noexcept;

    IAllocator* m_backingAllocator = nullptr;
    Block* m_blocks = nullptr;
    Block* m_current = nullptr;
    usize m_blockSize = 0;
    usize m_blockCount = 0;
    AllocatorStats m_stats;
};

}
