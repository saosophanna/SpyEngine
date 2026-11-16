#pragma once

#include <cstddef>

#include "core/memory/allocator.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::memory {

class LinearAllocator final : public IAllocator
{
public:
    explicit LinearAllocator(usize capacity);
    LinearAllocator(void* buffer, usize capacity) noexcept;
    ~LinearAllocator() override;

    [[nodiscard]] void* allocate(
        usize size, usize alignment = kDefaultAlignment) noexcept override;

    void deallocate(void* pointer) noexcept override;
    void reset() noexcept override;

    [[nodiscard]] usize usedMemory() const noexcept override;
    [[nodiscard]] usize totalMemory() const noexcept override;
    [[nodiscard]] AllocatorStats stats() const noexcept override;
    [[nodiscard]] AllocatorCapabilities capabilities() const noexcept override;

    [[nodiscard]] usize capacity() const noexcept;
    [[nodiscard]] usize offset() const noexcept;

    [[nodiscard]] void* data() noexcept;
    [[nodiscard]] const void* data() const noexcept;

private:
    void* m_memory = nullptr;
    usize m_capacity = 0;
    usize m_offset = 0;
    bool m_owned = false;
    AllocatorStats m_stats;
};

}
