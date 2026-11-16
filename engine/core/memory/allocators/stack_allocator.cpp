#include <cstddef>
#include <cstdint>

#include "core/memory/allocators/stack_allocator.hpp"

namespace spy::memory {

StackAllocator::StackAllocator(std::size_t capacity)
{
    if (capacity == 0)
    {
        return;
    }

    m_memory = allocateSystem(capacity, kDefaultAlignment);
    m_capacity = m_memory != nullptr ? capacity : 0;
    m_stats.totalMemory = m_capacity;
}

StackAllocator::~StackAllocator()
{
    deallocateSystem(m_memory);
}

void* StackAllocator::allocate(std::size_t size, std::size_t alignment) noexcept
{
    if (size == 0 || m_memory == nullptr || !isPowerOfTwo(alignment))
    {
        ++m_stats.failedAllocationCount;
        return nullptr;
    }

    const std::uintptr_t address = reinterpret_cast<std::uintptr_t>(m_memory) + m_offset;
    const std::size_t padding = alignmentPadding(address, alignment);

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

void StackAllocator::deallocate(void* pointer) noexcept
{
    if (pointer == nullptr || m_memory == nullptr)
    {
        return;
    }

    const std::uintptr_t base = reinterpret_cast<std::uintptr_t>(m_memory);
    const std::uintptr_t address = reinterpret_cast<std::uintptr_t>(pointer);

    if (address < base || address >= base + m_capacity)
    {
        return;
    }

    const std::size_t offset = static_cast<std::size_t>(address - base);

    if (offset >= m_offset)
    {
        return;
    }

    freeToMarker(offset);
    ++m_stats.deallocationCount;
}

void StackAllocator::reset() noexcept
{
    m_offset = 0;
    m_stats.usedMemory = 0;
}

std::size_t StackAllocator::usedMemory() const noexcept
{
    return m_offset;
}

std::size_t StackAllocator::totalMemory() const noexcept
{
    return m_capacity;
}

AllocatorStats StackAllocator::stats() const noexcept
{
    return m_stats;
}

AllocatorCapabilities StackAllocator::capabilities() const noexcept
{
    AllocatorCapabilities capabilities;
    capabilities.individualDeallocation = false;
    capabilities.reset = true;
    capabilities.stackRewind = true;
    capabilities.growth = false;
    return capabilities;
}

StackMarker StackAllocator::marker() const noexcept
{
    return m_offset;
}

void StackAllocator::freeToMarker(StackMarker marker) noexcept
{
    if (marker > m_offset)
    {
        return;
    }

    m_offset = marker;
    m_stats.usedMemory = marker;
}

std::size_t StackAllocator::capacity() const noexcept
{
    return m_capacity;
}

ScopedStackAllocation::ScopedStackAllocation(StackAllocator& allocator) noexcept
    : m_allocator(&allocator),
      m_marker(allocator.marker())
{
}

ScopedStackAllocation::~ScopedStackAllocation()
{
    if (m_allocator != nullptr)
    {
        m_allocator->freeToMarker(m_marker);
    }
}

ScopedStackAllocation::ScopedStackAllocation(ScopedStackAllocation&& other) noexcept
    : m_allocator(other.m_allocator),
      m_marker(other.m_marker)
{
    other.m_allocator = nullptr;
}

StackMarker ScopedStackAllocation::marker() const noexcept
{
    return m_marker;
}

}
