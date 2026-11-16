#pragma once

#include <cstddef>
#include <source_location>

#include "core/memory/allocation.hpp"

namespace spy::memory {

struct AllocationRecord
{
    void* address = nullptr;
    std::size_t size = 0;
    std::size_t alignment = 0;
    std::source_location location{};
};

class AllocationTracker
{
public:
    static constexpr std::size_t kDefaultCapacity = 1024;
    static constexpr std::size_t kReleaseHistoryCapacity = 16;

    explicit AllocationTracker(std::size_t capacity = kDefaultCapacity);
    ~AllocationTracker();

    AllocationTracker(const AllocationTracker&) = delete;
    AllocationTracker& operator=(const AllocationTracker&) = delete;

    [[nodiscard]] bool track(
        void* address,
        std::size_t size,
        std::size_t alignment = kDefaultAlignment,
        std::source_location location = std::source_location::current()) noexcept;

    [[nodiscard]] bool release(void* address) noexcept;

    [[nodiscard]] const AllocationRecord* find(void* address) const noexcept;
    [[nodiscard]] bool contains(void* address) const noexcept;

    [[nodiscard]] std::size_t liveCount() const noexcept;
    [[nodiscard]] std::size_t liveBytes() const noexcept;
    [[nodiscard]] std::size_t capacity() const noexcept;
    [[nodiscard]] std::size_t overflowCount() const noexcept;
    [[nodiscard]] std::size_t invalidReleaseCount() const noexcept;
    [[nodiscard]] bool doubleFreeDetected() const noexcept;

    void clear() noexcept;

private:
    [[nodiscard]] bool wasRecentlyReleased(const void* address) const noexcept;
    void recordReleaseHistory(void* address) noexcept;

    AllocationRecord* m_records = nullptr;
    std::size_t m_capacity = 0;
    std::size_t m_count = 0;
    std::size_t m_liveBytes = 0;
    std::size_t m_overflowCount = 0;
    std::size_t m_invalidReleaseCount = 0;
    bool m_doubleFreeDetected = false;

    void* m_releaseHistory[kReleaseHistoryCapacity] = {};
    std::size_t m_releaseHistoryCount = 0;
    std::size_t m_releaseHistoryNext = 0;
};

}
