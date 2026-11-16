#pragma once

#include <cstddef>

#include "core/memory/allocator.hpp"

namespace spy::memory {

using StackMarker = std::size_t;

class StackAllocator final : public IAllocator
{
public:
    explicit StackAllocator(std::size_t capacity);
    ~StackAllocator() override;

    StackAllocator(const StackAllocator&) = delete;
    StackAllocator& operator=(const StackAllocator&) = delete;

    [[nodiscard]] void* allocate(
        std::size_t size, std::size_t alignment = kDefaultAlignment) noexcept override;

    void deallocate(void* pointer) noexcept override;
    void reset() noexcept override;

    [[nodiscard]] std::size_t usedMemory() const noexcept override;
    [[nodiscard]] std::size_t totalMemory() const noexcept override;
    [[nodiscard]] AllocatorStats stats() const noexcept override;
    [[nodiscard]] AllocatorCapabilities capabilities() const noexcept override;

    [[nodiscard]] StackMarker marker() const noexcept;
    void freeToMarker(StackMarker marker) noexcept;

    [[nodiscard]] std::size_t capacity() const noexcept;

private:
    void* m_memory = nullptr;
    std::size_t m_capacity = 0;
    std::size_t m_offset = 0;
    AllocatorStats m_stats;
};

class ScopedStackAllocation
{
public:
    explicit ScopedStackAllocation(StackAllocator& allocator) noexcept;
    ~ScopedStackAllocation();

    ScopedStackAllocation(const ScopedStackAllocation&) = delete;
    ScopedStackAllocation& operator=(const ScopedStackAllocation&) = delete;

    ScopedStackAllocation(ScopedStackAllocation&& other) noexcept;
    ScopedStackAllocation& operator=(ScopedStackAllocation&&) = delete;

    [[nodiscard]] StackMarker marker() const noexcept;

private:
    StackAllocator* m_allocator = nullptr;
    StackMarker m_marker = 0;
};

}
