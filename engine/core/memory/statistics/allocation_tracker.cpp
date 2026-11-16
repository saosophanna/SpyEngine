#include <cstddef>
#include <limits>

#include "core/memory/statistics/allocation_tracker.hpp"

namespace spy::memory {

AllocationTracker::AllocationTracker(std::size_t capacity)
{
    if (capacity == 0 ||
        capacity > std::numeric_limits<std::size_t>::max() / sizeof(AllocationRecord))
    {
        return;
    }

    m_records = static_cast<AllocationRecord*>(
        allocateSystem(capacity * sizeof(AllocationRecord), alignof(AllocationRecord)));

    if (m_records == nullptr)
    {
        return;
    }

    m_capacity = capacity;
}

AllocationTracker::~AllocationTracker()
{
    deallocateSystem(m_records);
}

bool AllocationTracker::track(
    void* address,
    std::size_t size,
    std::size_t alignment,
    std::source_location location) noexcept
{
    if (address == nullptr || m_count >= m_capacity || contains(address))
    {
        ++m_overflowCount;
        return false;
    }

    AllocationRecord& record = m_records[m_count];
    record.address = address;
    record.size = size;
    record.alignment = alignment;
    record.location = location;

    ++m_count;
    m_liveBytes += size;

    return true;
}

bool AllocationTracker::release(void* address) noexcept
{
    if (address == nullptr)
    {
        return false;
    }

    for (std::size_t index = 0; index < m_count; ++index)
    {
        if (m_records[index].address != address)
        {
            continue;
        }

        m_liveBytes -= m_records[index].size;
        m_records[index] = m_records[m_count - 1];
        --m_count;
        recordReleaseHistory(address);

        return true;
    }

    ++m_invalidReleaseCount;

    if (wasRecentlyReleased(address))
    {
        m_doubleFreeDetected = true;
    }

    return false;
}

const AllocationRecord* AllocationTracker::find(void* address) const noexcept
{
    for (std::size_t index = 0; index < m_count; ++index)
    {
        if (m_records[index].address == address)
        {
            return &m_records[index];
        }
    }

    return nullptr;
}

bool AllocationTracker::contains(void* address) const noexcept
{
    return find(address) != nullptr;
}

std::size_t AllocationTracker::liveCount() const noexcept
{
    return m_count;
}

std::size_t AllocationTracker::liveBytes() const noexcept
{
    return m_liveBytes;
}

std::size_t AllocationTracker::capacity() const noexcept
{
    return m_capacity;
}

std::size_t AllocationTracker::overflowCount() const noexcept
{
    return m_overflowCount;
}

std::size_t AllocationTracker::invalidReleaseCount() const noexcept
{
    return m_invalidReleaseCount;
}

bool AllocationTracker::doubleFreeDetected() const noexcept
{
    return m_doubleFreeDetected;
}

void AllocationTracker::clear() noexcept
{
    m_count = 0;
    m_liveBytes = 0;
    m_overflowCount = 0;
    m_invalidReleaseCount = 0;
    m_doubleFreeDetected = false;
    m_releaseHistoryCount = 0;
    m_releaseHistoryNext = 0;

    for (std::size_t index = 0; index < kReleaseHistoryCapacity; ++index)
    {
        m_releaseHistory[index] = nullptr;
    }
}

bool AllocationTracker::wasRecentlyReleased(const void* address) const noexcept
{
    for (std::size_t index = 0; index < m_releaseHistoryCount; ++index)
    {
        if (m_releaseHistory[index] == address)
        {
            return true;
        }
    }

    return false;
}

void AllocationTracker::recordReleaseHistory(void* address) noexcept
{
    m_releaseHistory[m_releaseHistoryNext] = address;
    m_releaseHistoryNext = (m_releaseHistoryNext + 1) % kReleaseHistoryCapacity;

    if (m_releaseHistoryCount < kReleaseHistoryCapacity)
    {
        ++m_releaseHistoryCount;
    }
}

}
