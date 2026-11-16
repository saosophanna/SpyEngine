#include <cstddef>
#include <cstdint>
#include <limits>
#include <new>

#include "core/memory/allocators/pool_allocator.hpp"

namespace spy::memory {

namespace {

std::size_t normalizeObjectSize(std::size_t objectSize) noexcept
{
    return objectSize == 0 ? sizeof(void*) : objectSize;
}

std::size_t normalizeObjectAlignment(std::size_t objectAlignment) noexcept
{
    const bool valid = isPowerOfTwo(objectAlignment) && objectAlignment >= alignof(void*);
    return valid ? objectAlignment : alignof(void*);
}

std::size_t computeStride(std::size_t objectSize, std::size_t objectAlignment) noexcept
{
    const std::size_t payload = objectSize > sizeof(void*) ? objectSize : sizeof(void*);
    return alignUp(payload, objectAlignment);
}

}

PoolAllocator::PoolAllocator(
    std::size_t objectSize,
    std::size_t objectAlignment,
    std::size_t capacity,
    IAllocator* backingAllocator)
    : m_backingAllocator(backingAllocator),
      m_objectSize(normalizeObjectSize(objectSize)),
      m_objectAlignment(normalizeObjectAlignment(objectAlignment)),
      m_stride(computeStride(
          normalizeObjectSize(objectSize), normalizeObjectAlignment(objectAlignment))),
      m_capacity(capacity)
{
    if (m_capacity != 0 && m_stride > std::numeric_limits<std::size_t>::max() / m_capacity)
    {
        m_capacity = 0;
        m_freeCount = 0;
        return;
    }

    if (m_capacity == 0)
    {
        return;
    }

    void* memory = allocateBacking(m_stride * m_capacity, m_objectAlignment);

    if (memory == nullptr)
    {
        m_capacity = 0;
        return;
    }

    m_memory = static_cast<std::byte*>(memory);
    m_stats.totalMemory = m_stride * m_capacity;
    rebuildFreeList();
}

PoolAllocator::~PoolAllocator()
{
    deallocateBacking(m_memory);
}

void* PoolAllocator::allocate(std::size_t size, std::size_t alignment) noexcept
{
    if (size == 0 || size > m_objectSize || !isPowerOfTwo(alignment) ||
        alignment > m_objectAlignment || m_freeList == nullptr)
    {
        ++m_stats.failedAllocationCount;
        return nullptr;
    }

    void* block = m_freeList;
    m_freeList = *static_cast<void**>(block);
    --m_freeCount;
    ++m_stats.allocationCount;
    updateUsedMemory();

    return block;
}

void* PoolAllocator::allocate() noexcept
{
    return allocate(m_objectSize, m_objectAlignment);
}

void PoolAllocator::deallocate(void* pointer) noexcept
{
    if (pointer == nullptr || m_memory == nullptr)
    {
        return;
    }

    const std::uintptr_t base = reinterpret_cast<std::uintptr_t>(m_memory);
    const std::uintptr_t address = reinterpret_cast<std::uintptr_t>(pointer);
    const std::uintptr_t end = base + (m_stride * m_capacity);

    if (address < base || address >= end)
    {
        return;
    }

    const std::size_t offset = static_cast<std::size_t>(address - base);

    if (offset % m_stride != 0 || isFreeBlock(pointer))
    {
        return;
    }

    *static_cast<void**>(pointer) = m_freeList;
    m_freeList = pointer;
    ++m_freeCount;
    ++m_stats.deallocationCount;
    updateUsedMemory();
}

void PoolAllocator::reset() noexcept
{
    rebuildFreeList();
    m_stats.usedMemory = 0;
}

std::size_t PoolAllocator::usedMemory() const noexcept
{
    return m_stats.usedMemory;
}

std::size_t PoolAllocator::totalMemory() const noexcept
{
    return m_stride * m_capacity;
}

AllocatorStats PoolAllocator::stats() const noexcept
{
    return m_stats;
}

AllocatorCapabilities PoolAllocator::capabilities() const noexcept
{
    AllocatorCapabilities capabilities;
    capabilities.individualDeallocation = true;
    capabilities.reset = true;
    capabilities.stackRewind = false;
    capabilities.growth = false;
    return capabilities;
}

std::size_t PoolAllocator::objectSize() const noexcept
{
    return m_objectSize;
}

std::size_t PoolAllocator::objectAlignment() const noexcept
{
    return m_objectAlignment;
}

std::size_t PoolAllocator::stride() const noexcept
{
    return m_stride;
}

std::size_t PoolAllocator::capacity() const noexcept
{
    return m_capacity;
}

std::size_t PoolAllocator::freeCount() const noexcept
{
    return m_freeCount;
}

void* PoolAllocator::allocateBacking(std::size_t size, std::size_t alignment) noexcept
{
    if (m_backingAllocator != nullptr)
    {
        return m_backingAllocator->allocate(size, alignment);
    }

    return allocateSystem(size, alignment);
}

void PoolAllocator::deallocateBacking(void* pointer) noexcept
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

bool PoolAllocator::isFreeBlock(const void* block) const noexcept
{
    for (const void* freeBlock = m_freeList; freeBlock != nullptr;
         freeBlock = *static_cast<void* const*>(freeBlock))
    {
        if (freeBlock == block)
        {
            return true;
        }
    }

    return false;
}

void PoolAllocator::rebuildFreeList() noexcept
{
    m_freeList = nullptr;
    m_freeCount = m_capacity;

    for (std::size_t index = m_capacity; index > 0; --index)
    {
        const std::size_t blockIndex = index - 1;
        void* block = m_memory + (blockIndex * m_stride);
        *static_cast<void**>(block) = m_freeList;
        m_freeList = block;
    }
}

void PoolAllocator::updateUsedMemory() noexcept
{
    m_stats.usedMemory = (m_capacity - m_freeCount) * m_stride;

    if (m_stats.usedMemory > m_stats.peakMemory)
    {
        m_stats.peakMemory = m_stats.usedMemory;
    }
}

}
