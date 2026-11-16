#pragma once

#include <cstddef>
#include <source_location>

#include "core/memory/allocation.hpp"
#include "core/utilities/memory/non_copyable.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::memory {

struct AllocationRecord
{
    void* address = nullptr;
    usize size = 0;
    usize alignment = 0;
    std::source_location location{};
};

class AllocationTracker : public NonCopyable
{
public:
    static constexpr usize kDefaultCapacity = 1024;
    static constexpr usize kReleaseHistoryCapacity = 16;

    explicit AllocationTracker(usize capacity = kDefaultCapacity);
    ~AllocationTracker();

    [[nodiscard]] bool track(
        void* address,
        usize size,
        usize alignment = kDefaultAlignment,
        std::source_location location = std::source_location::current()) noexcept;

    [[nodiscard]] bool release(void* address) noexcept;

    [[nodiscard]] const AllocationRecord* find(void* address) const noexcept;
    [[nodiscard]] bool contains(void* address) const noexcept;

    [[nodiscard]] usize liveCount() const noexcept;
    [[nodiscard]] usize liveBytes() const noexcept;
    [[nodiscard]] usize capacity() const noexcept;
    [[nodiscard]] usize overflowCount() const noexcept;
    [[nodiscard]] usize invalidReleaseCount() const noexcept;
    [[nodiscard]] bool doubleFreeDetected() const noexcept;

    void clear() noexcept;

private:
    [[nodiscard]] bool wasRecentlyReleased(const void* address) const noexcept;
    void recordReleaseHistory(void* address) noexcept;

    AllocationRecord* m_records = nullptr;
    usize m_capacity = 0;
    usize m_count = 0;
    usize m_liveBytes = 0;
    usize m_overflowCount = 0;
    usize m_invalidReleaseCount = 0;
    bool m_doubleFreeDetected = false;

    void* m_releaseHistory[kReleaseHistoryCapacity] = {};
    usize m_releaseHistoryCount = 0;
    usize m_releaseHistoryNext = 0;
};

}
