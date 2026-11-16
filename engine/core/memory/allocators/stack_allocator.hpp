#pragma once

#include <cstddef>

#include "core/memory/allocator.hpp"
#include "core/utilities/memory/non_copyable.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::memory {

using StackMarker = usize;

class StackAllocator final : public IAllocator
{
public:
    explicit StackAllocator(usize capacity);
    ~StackAllocator() override;

    [[nodiscard]] void* allocate(
        usize size, usize alignment = kDefaultAlignment) noexcept override;

    void deallocate(void* pointer) noexcept override;
    void reset() noexcept override;

    [[nodiscard]] usize usedMemory() const noexcept override;
    [[nodiscard]] usize totalMemory() const noexcept override;
    [[nodiscard]] AllocatorStats stats() const noexcept override;
    [[nodiscard]] AllocatorCapabilities capabilities() const noexcept override;

    [[nodiscard]] StackMarker marker() const noexcept;
    void freeToMarker(StackMarker marker) noexcept;

    [[nodiscard]] usize capacity() const noexcept;

private:
    void* m_memory = nullptr;
    usize m_capacity = 0;
    usize m_offset = 0;
    AllocatorStats m_stats;
};

class ScopedStackAllocation : public NonCopyable
{
public:
    explicit ScopedStackAllocation(StackAllocator& allocator) noexcept;
    ~ScopedStackAllocation();

    ScopedStackAllocation(ScopedStackAllocation&& other) noexcept;
    ScopedStackAllocation& operator=(ScopedStackAllocation&&) = delete;

    [[nodiscard]] StackMarker marker() const noexcept;

private:
    StackAllocator* m_allocator = nullptr;
    StackMarker m_marker = 0;
};

}
