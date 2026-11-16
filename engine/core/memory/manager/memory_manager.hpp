#pragma once

#include <cstddef>
#include <cstdint>

#include "core/memory/allocator.hpp"
#include "core/memory/statistics/memory_stats.hpp"

namespace spy::memory {

class MemoryManager
{
public:
    static constexpr std::size_t kMaxAllocators = 16;

    MemoryManager() = default;

    MemoryManager(const MemoryManager&) = delete;
    MemoryManager& operator=(const MemoryManager&) = delete;

    [[nodiscard]] bool initialize() noexcept;
    void shutdown() noexcept;

    [[nodiscard]] bool registerAllocator(std::uint32_t id, IAllocator& allocator) noexcept;
    [[nodiscard]] bool unregisterAllocator(std::uint32_t id) noexcept;

    [[nodiscard]] IAllocator* allocator(std::uint32_t id) noexcept;
    [[nodiscard]] const IAllocator* allocator(std::uint32_t id) const noexcept;

    [[nodiscard]] std::uint32_t allocatorCount() const noexcept;
    [[nodiscard]] MemoryStats stats() const noexcept;

    void resetAllocators() noexcept;

private:
    struct Entry
    {
        std::uint32_t id = 0;
        IAllocator* allocator = nullptr;
    };

    [[nodiscard]] Entry* find(std::uint32_t id) noexcept;

    Entry m_entries[kMaxAllocators];
    std::uint32_t m_count = 0;
    bool m_initialized = false;
};

}
