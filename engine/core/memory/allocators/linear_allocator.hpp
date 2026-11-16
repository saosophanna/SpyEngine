#pragma once

#include <cstddef>

#include "core/memory/allocator.hpp"

namespace spy::memory {

class LinearAllocator final : public IAllocator
{
public:
    explicit LinearAllocator(std::size_t capacity);
    LinearAllocator(void* buffer, std::size_t capacity) noexcept;
    ~LinearAllocator() override;

    LinearAllocator(const LinearAllocator&) = delete;
    LinearAllocator& operator=(const LinearAllocator&) = delete;

    [[nodiscard]] void* allocate(
        std::size_t size, std::size_t alignment = kDefaultAlignment) noexcept override;

    void deallocate(void* pointer) noexcept override;
    void reset() noexcept override;

    [[nodiscard]] std::size_t usedMemory() const noexcept override;
    [[nodiscard]] std::size_t totalMemory() const noexcept override;
    [[nodiscard]] AllocatorStats stats() const noexcept override;
    [[nodiscard]] AllocatorCapabilities capabilities() const noexcept override;

    [[nodiscard]] std::size_t capacity() const noexcept;
    [[nodiscard]] std::size_t offset() const noexcept;

    [[nodiscard]] void* data() noexcept;
    [[nodiscard]] const void* data() const noexcept;

private:
    void* m_memory = nullptr;
    std::size_t m_capacity = 0;
    std::size_t m_offset = 0;
    bool m_owned = false;
    AllocatorStats m_stats;
};

}
