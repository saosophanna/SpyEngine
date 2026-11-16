#pragma once

#include <cstddef>
#include <cstdint>

#include "core/memory/allocator.hpp"
#include "core/memory/statistics/memory_stats.hpp"
#include "core/utilities/memory/non_copyable.hpp"
#include "core/utilities/types/id.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::memory {

struct AllocatorTag;

using AllocatorId = Id<AllocatorTag>;

class MemoryManager : public NonCopyable
{
public:
    static constexpr usize kMaxAllocators = 16;

    MemoryManager() = default;

    [[nodiscard]] bool initialize() noexcept;
    void shutdown() noexcept;

    [[nodiscard]] bool registerAllocator(AllocatorId id, IAllocator& allocator) noexcept;
    [[nodiscard]] bool unregisterAllocator(AllocatorId id) noexcept;

    [[nodiscard]] IAllocator* allocator(AllocatorId id) noexcept;
    [[nodiscard]] const IAllocator* allocator(AllocatorId id) const noexcept;

    [[nodiscard]] uint32 allocatorCount() const noexcept;
    [[nodiscard]] MemoryStats stats() const noexcept;

    void resetAllocators() noexcept;

private:
    struct Entry
    {
        AllocatorId id{};
        IAllocator* allocator = nullptr;
    };

    [[nodiscard]] Entry* find(AllocatorId id) noexcept;

    Entry m_entries[kMaxAllocators];
    usize m_count = 0;
    bool m_initialized = false;
};

}
