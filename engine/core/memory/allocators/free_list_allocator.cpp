#include <cstddef>
#include <cstdint>
#include <new>

#include "core/memory/allocators/free_list_allocator.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::memory {

namespace {

constexpr usize kMinRemainderSize = sizeof(void*);
constexpr usize kAllocationMagic = 0x5350594Du;

}

struct FreeListAllocator::BlockHeader
{
    usize size = 0;
    BlockHeader* previous = nullptr;
    BlockHeader* next = nullptr;
    BlockHeader* previousFree = nullptr;
    BlockHeader* nextFree = nullptr;
    bool free = false;
};

struct FreeListAllocator::AllocationHeader
{
    BlockHeader* block = nullptr;
    void* pointer = nullptr;
    usize magic = 0;
};

FreeListAllocator::FreeListAllocator(usize capacity, IAllocator* backingAllocator)
    : m_backingAllocator(backingAllocator)
{
    if (capacity < sizeof(BlockHeader) + sizeof(AllocationHeader) + kMinRemainderSize)
    {
        return;
    }

    m_memory = static_cast<byte*>(allocateBacking(capacity, alignof(BlockHeader)));

    if (m_memory == nullptr)
    {
        return;
    }

    m_capacity = capacity;
    m_stats.totalMemory = capacity;
    initializeBlock();
}

FreeListAllocator::~FreeListAllocator()
{
    deallocateBacking(m_memory);
}

void* FreeListAllocator::allocate(usize size, usize alignment) noexcept
{
    if (size == 0 || !isPowerOfTwo(alignment) || m_freeList == nullptr)
    {
        ++m_stats.failedAllocationCount;
        return nullptr;
    }

    const usize effectiveAlignment =
        alignment < alignof(AllocationHeader) ? alignof(AllocationHeader) : alignment;

    for (BlockHeader* block = m_freeList; block != nullptr; block = block->nextFree)
    {
        const std::uintptr_t blockAddress = reinterpret_cast<std::uintptr_t>(block);
        const usize headerSize = sizeof(BlockHeader) + sizeof(AllocationHeader);

        if (block->size < headerSize)
        {
            continue;
        }

        const std::uintptr_t payloadAddress = blockAddress + headerSize;
        const usize padding = alignmentPadding(payloadAddress, effectiveAlignment);
        const usize available = block->size - headerSize;

        if (padding > available || size > available - padding)
        {
            continue;
        }

        const std::uintptr_t payload = payloadAddress + padding;
        const std::uintptr_t payloadEnd = payload + size;
        const std::uintptr_t blockEnd = blockAddress + block->size;
        const std::uintptr_t remainderAddress = alignUp(payloadEnd, alignof(BlockHeader));

        removeFreeBlock(block);

        usize usedSize = block->size;

        if (remainderAddress < blockEnd &&
            blockEnd - remainderAddress >= sizeof(BlockHeader) + kMinRemainderSize)
        {
            const usize remainderSize = static_cast<usize>(blockEnd - remainderAddress);
            usedSize = static_cast<usize>(remainderAddress - blockAddress);

            auto* remainder = ::new (reinterpret_cast<void*>(remainderAddress)) BlockHeader{};
            remainder->size = remainderSize;
            remainder->free = true;
            remainder->previous = block;
            remainder->next = block->next;

            if (remainder->next != nullptr)
            {
                remainder->next->previous = remainder;
            }

            block->next = remainder;
            block->size = usedSize;
            insertFreeBlock(remainder);
        }

        block->free = false;

        auto* allocation = reinterpret_cast<AllocationHeader*>(payload) - 1;
        allocation->block = block;
        allocation->pointer = reinterpret_cast<void*>(payload);
        allocation->magic = kAllocationMagic;

        ++m_stats.allocationCount;
        m_stats.usedMemory += usedSize;

        if (m_stats.usedMemory > m_stats.peakMemory)
        {
            m_stats.peakMemory = m_stats.usedMemory;
        }

        return reinterpret_cast<void*>(payload);
    }

    ++m_stats.failedAllocationCount;
    return nullptr;
}

void FreeListAllocator::deallocate(void* pointer) noexcept
{
    if (pointer == nullptr || m_memory == nullptr)
    {
        return;
    }

    const std::uintptr_t begin = reinterpret_cast<std::uintptr_t>(m_memory);
    const std::uintptr_t end = begin + m_capacity;
    const std::uintptr_t address = reinterpret_cast<std::uintptr_t>(pointer);

    if (address < begin + sizeof(BlockHeader) + sizeof(AllocationHeader) || address >= end)
    {
        return;
    }

    auto* allocation = static_cast<AllocationHeader*>(pointer) - 1;

    if (allocation->magic != kAllocationMagic || allocation->pointer != pointer)
    {
        return;
    }

    BlockHeader* block = allocation->block;

    if (block == nullptr || block->free)
    {
        return;
    }

    const std::uintptr_t blockAddress = reinterpret_cast<std::uintptr_t>(block);

    if (blockAddress < begin || blockAddress >= end || blockAddress + block->size > end)
    {
        return;
    }

    if (address < blockAddress + sizeof(BlockHeader) || address >= blockAddress + block->size)
    {
        return;
    }

    allocation->magic = 0;
    allocation->pointer = nullptr;

    m_stats.usedMemory -= block->size;
    ++m_stats.deallocationCount;
    block->free = true;

    BlockHeader* next = block->next;

    if (next != nullptr && next->free)
    {
        removeFreeBlock(next);
        block->size += next->size;
        block->next = next->next;

        if (block->next != nullptr)
        {
            block->next->previous = block;
        }
    }

    BlockHeader* previous = block->previous;

    if (previous != nullptr && previous->free)
    {
        removeFreeBlock(previous);
        previous->size += block->size;
        previous->next = block->next;

        if (previous->next != nullptr)
        {
            previous->next->previous = previous;
        }

        block = previous;
    }

    insertFreeBlock(block);
}

void FreeListAllocator::reset() noexcept
{
    if (m_memory == nullptr)
    {
        return;
    }

    initializeBlock();
    m_stats.usedMemory = 0;
}

usize FreeListAllocator::usedMemory() const noexcept
{
    return m_stats.usedMemory;
}

usize FreeListAllocator::totalMemory() const noexcept
{
    return m_capacity;
}

AllocatorStats FreeListAllocator::stats() const noexcept
{
    return m_stats;
}

AllocatorCapabilities FreeListAllocator::capabilities() const noexcept
{
    AllocatorCapabilities capabilities;
    capabilities.individualDeallocation = true;
    capabilities.reset = true;
    capabilities.stackRewind = false;
    capabilities.growth = false;
    return capabilities;
}

usize FreeListAllocator::capacity() const noexcept
{
    return m_capacity;
}

usize FreeListAllocator::freeMemory() const noexcept
{
    usize total = 0;

    for (const BlockHeader* block = m_freeList; block != nullptr; block = block->nextFree)
    {
        total += block->size;
    }

    return total;
}

usize FreeListAllocator::largestFreeBlock() const noexcept
{
    usize largest = 0;

    for (const BlockHeader* block = m_freeList; block != nullptr; block = block->nextFree)
    {
        if (block->size > largest)
        {
            largest = block->size;
        }
    }

    return largest;
}

void* FreeListAllocator::allocateBacking(usize size, usize alignment) noexcept
{
    if (m_backingAllocator != nullptr)
    {
        return m_backingAllocator->allocate(size, alignment);
    }

    return allocateSystem(size, alignment);
}

void FreeListAllocator::deallocateBacking(void* pointer) noexcept
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

void FreeListAllocator::initializeBlock() noexcept
{
    m_firstBlock = ::new (m_memory) BlockHeader{};
    m_firstBlock->size = m_capacity;
    m_firstBlock->free = true;
    m_freeList = m_firstBlock;
}

void FreeListAllocator::removeFreeBlock(BlockHeader* block) noexcept
{
    if (block->previousFree != nullptr)
    {
        block->previousFree->nextFree = block->nextFree;
    }
    else
    {
        m_freeList = block->nextFree;
    }

    if (block->nextFree != nullptr)
    {
        block->nextFree->previousFree = block->previousFree;
    }

    block->previousFree = nullptr;
    block->nextFree = nullptr;
}

void FreeListAllocator::insertFreeBlock(BlockHeader* block) noexcept
{
    block->previousFree = nullptr;
    block->nextFree = m_freeList;

    if (m_freeList != nullptr)
    {
        m_freeList->previousFree = block;
    }

    m_freeList = block;
}

}
