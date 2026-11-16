#include <cstddef>
#include <cstdint>
#include <limits>
#include <new>

#include "core/memory/allocators/arena_allocator.hpp"

namespace spy::memory {

ArenaAllocator::ArenaAllocator(std::size_t blockSize, IAllocator* backingAllocator)
    : m_backingAllocator(backingAllocator),
      m_blockSize(blockSize == 0 ? kDefaultBlockSize : blockSize)
{
}

ArenaAllocator::~ArenaAllocator()
{
    release();
}

void* ArenaAllocator::allocate(std::size_t size, std::size_t alignment) noexcept
{
    if (size == 0 || !isPowerOfTwo(alignment))
    {
        ++m_stats.failedAllocationCount;
        return nullptr;
    }

    if (m_current != nullptr)
    {
        std::byte* current = m_current->current;
        const std::size_t padding =
            alignmentPadding(reinterpret_cast<std::uintptr_t>(current), alignment);
        const std::size_t available = static_cast<std::size_t>(m_current->end - current);

        if (padding <= available && size <= available - padding)
        {
            m_current->current = current + padding + size;
            recordAllocation(padding + size);
            return current + padding;
        }
    }

    if (size > std::numeric_limits<std::size_t>::max() - alignment)
    {
        ++m_stats.failedAllocationCount;
        return nullptr;
    }

    Block* block = createBlock(size + alignment);

    if (block == nullptr)
    {
        ++m_stats.failedAllocationCount;
        return nullptr;
    }

    std::byte* current = block->current;
    const std::size_t padding =
        alignmentPadding(reinterpret_cast<std::uintptr_t>(current), alignment);

    block->current = current + padding + size;
    recordAllocation(padding + size);

    return current + padding;
}

void ArenaAllocator::deallocate(void*) noexcept
{
}

void ArenaAllocator::reset() noexcept
{
    for (Block* block = m_blocks; block != nullptr; block = block->next)
    {
        block->current = block->begin;
    }

    m_current = m_blocks;
    m_stats.usedMemory = 0;
}

std::size_t ArenaAllocator::usedMemory() const noexcept
{
    return m_stats.usedMemory;
}

std::size_t ArenaAllocator::totalMemory() const noexcept
{
    return m_stats.totalMemory;
}

AllocatorStats ArenaAllocator::stats() const noexcept
{
    return m_stats;
}

AllocatorCapabilities ArenaAllocator::capabilities() const noexcept
{
    AllocatorCapabilities capabilities;
    capabilities.individualDeallocation = false;
    capabilities.reset = true;
    capabilities.stackRewind = false;
    capabilities.growth = true;
    return capabilities;
}

std::size_t ArenaAllocator::blockSize() const noexcept
{
    return m_blockSize;
}

std::size_t ArenaAllocator::blockCount() const noexcept
{
    return m_blockCount;
}

void ArenaAllocator::release() noexcept
{
    Block* block = m_blocks;

    while (block != nullptr)
    {
        Block* next = block->next;
        deallocateBacking(block);
        block = next;
    }

    m_blocks = nullptr;
    m_current = nullptr;
    m_blockCount = 0;
    m_stats.totalMemory = 0;
    m_stats.usedMemory = 0;
}

void* ArenaAllocator::allocateBacking(std::size_t size, std::size_t alignment) noexcept
{
    if (m_backingAllocator != nullptr)
    {
        return m_backingAllocator->allocate(size, alignment);
    }

    return allocateSystem(size, alignment);
}

void ArenaAllocator::deallocateBacking(void* pointer) noexcept
{
    if (pointer == nullptr)
    {
        return;
    }

    if (m_backingAllocator != nullptr)
    {
        m_backingAllocator->deallocate(pointer);
        return;
    }

    deallocateSystem(pointer);
}

ArenaAllocator::Block* ArenaAllocator::createBlock(std::size_t payloadSize) noexcept
{
    const std::size_t capacity = payloadSize > m_blockSize ? payloadSize : m_blockSize;

    if (capacity > std::numeric_limits<std::size_t>::max() - sizeof(Block))
    {
        return nullptr;
    }

    void* memory = allocateBacking(capacity + sizeof(Block), alignof(Block));

    if (memory == nullptr)
    {
        return nullptr;
    }

    auto* block = ::new (memory) Block{};
    block->next = m_blocks;
    block->begin = reinterpret_cast<std::byte*>(memory) + sizeof(Block);
    block->current = block->begin;
    block->end = block->begin + capacity;

    m_blocks = block;
    m_current = block;
    ++m_blockCount;
    m_stats.totalMemory += capacity + sizeof(Block);

    return block;
}

void ArenaAllocator::recordAllocation(std::size_t bytes) noexcept
{
    ++m_stats.allocationCount;
    m_stats.usedMemory += bytes;

    if (m_stats.usedMemory > m_stats.peakMemory)
    {
        m_stats.peakMemory = m_stats.usedMemory;
    }
}

}
