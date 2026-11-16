#include <cstddef>
#include <cstdint>

#include "core/memory/allocators/linear_allocator.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::memory {

LinearAllocator::LinearAllocator(usize capacity)
{
    if (capacity == 0)
    {
        return;
    }

    m_memory = allocateSystem(capacity, kDefaultAlignment);
    m_capacity = m_memory != nullptr ? capacity : 0;
    m_owned = true;
    m_stats.totalMemory = m_capacity;
}

LinearAllocator::LinearAllocator(void* buffer, usize capacity) noexcept
{
    if (buffer == nullptr || capacity == 0)
    {
        return;
    }

    m_memory = buffer;
    m_capacity = capacity;
    m_stats.totalMemory = capacity;
}

LinearAllocator::~LinearAllocator()
{
    if (m_owned)
    {
        deallocateSystem(m_memory);
    }
}

void* LinearAllocator::allocate(usize size, usize alignment) noexcept
{
    if (size == 0 || m_memory == nullptr || !isPowerOfTwo(alignment))
    {
        ++m_stats.failedAllocationCount;
        return nullptr;
    }

    const std::uintptr_t address = reinterpret_cast<std::uintptr_t>(m_memory) + m_offset;
    const usize padding = alignmentPadding(address, alignment);

    if (padding > m_capacity - m_offset || size > m_capacity - m_offset - padding)
    {
        ++m_stats.failedAllocationCount;
        return nullptr;
    }

    m_offset += padding + size;
    ++m_stats.allocationCount;
    m_stats.usedMemory = m_offset;

    if (m_offset > m_stats.peakMemory)
    {
        m_stats.peakMemory = m_offset;
    }

    return reinterpret_cast<void*>(address + padding);
}

void LinearAllocator::deallocate(void*) noexcept
{
}

void LinearAllocator::reset() noexcept
{
    m_offset = 0;
    m_stats.usedMemory = 0;
}

usize LinearAllocator::usedMemory() const noexcept
{
    return m_offset;
}

usize LinearAllocator::totalMemory() const noexcept
{
    return m_capacity;
}

AllocatorStats LinearAllocator::stats() const noexcept
{
    return m_stats;
}

AllocatorCapabilities LinearAllocator::capabilities() const noexcept
{
    AllocatorCapabilities capabilities;
    capabilities.individualDeallocation = false;
    capabilities.reset = true;
    capabilities.stackRewind = false;
    capabilities.growth = false;
    return capabilities;
}

usize LinearAllocator::capacity() const noexcept
{
    return m_capacity;
}

usize LinearAllocator::offset() const noexcept
{
    return m_offset;
}

void* LinearAllocator::data() noexcept
{
    return m_memory;
}

const void* LinearAllocator::data() const noexcept
{
    return m_memory;
}

}
