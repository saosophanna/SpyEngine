#include <cstdint>

#include "core/memory/manager/memory_manager.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::memory {

bool MemoryManager::initialize() noexcept
{
    m_count = 0;
    m_initialized = true;
    return true;
}

void MemoryManager::shutdown() noexcept
{
    m_count = 0;
    m_initialized = false;
}

bool MemoryManager::registerAllocator(AllocatorId id, IAllocator& allocator) noexcept
{
    if (!m_initialized || !id.isValid() || m_count >= kMaxAllocators || find(id) != nullptr)
    {
        return false;
    }

    m_entries[m_count] = Entry{id, &allocator};
    ++m_count;

    return true;
}

bool MemoryManager::unregisterAllocator(AllocatorId id) noexcept
{
    Entry* entry = find(id);

    if (entry == nullptr)
    {
        return false;
    }

    *entry = m_entries[m_count - 1];
    --m_count;

    return true;
}

IAllocator* MemoryManager::allocator(AllocatorId id) noexcept
{
    Entry* entry = find(id);
    return entry != nullptr ? entry->allocator : nullptr;
}

const IAllocator* MemoryManager::allocator(AllocatorId id) const noexcept
{
    for (usize index = 0; index < m_count; ++index)
    {
        if (m_entries[index].id == id)
        {
            return m_entries[index].allocator;
        }
    }

    return nullptr;
}

uint32 MemoryManager::allocatorCount() const noexcept
{
    return static_cast<uint32>(m_count);
}

MemoryStats MemoryManager::stats() const noexcept
{
    MemoryStats result;

    for (usize index = 0; index < m_count; ++index)
    {
        const AllocatorStats allocatorStats = m_entries[index].allocator->stats();

        result.totalMemory += allocatorStats.totalMemory;
        result.usedMemory += allocatorStats.usedMemory;
        result.peakMemory += allocatorStats.peakMemory;
        result.allocationCount += allocatorStats.allocationCount;
        result.deallocationCount += allocatorStats.deallocationCount;
        result.failedAllocationCount += allocatorStats.failedAllocationCount;
    }

    result.allocatorCount = static_cast<uint32>(m_count);

    return result;
}

void MemoryManager::resetAllocators() noexcept
{
    for (usize index = 0; index < m_count; ++index)
    {
        m_entries[index].allocator->reset();
    }
}

MemoryManager::Entry* MemoryManager::find(AllocatorId id) noexcept
{
    for (usize index = 0; index < m_count; ++index)
    {
        if (m_entries[index].id == id)
        {
            return &m_entries[index];
        }
    }

    return nullptr;
}

}
