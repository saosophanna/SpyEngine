#pragma once

#include <cstddef>
#include <memory_resource>

#include "core/memory/allocator.hpp"

namespace spy::memory {

class AllocatorResource final : public std::pmr::memory_resource
{
public:
    explicit AllocatorResource(IAllocator& allocator) noexcept;
    ~AllocatorResource() override = default;

    [[nodiscard]] IAllocator* allocator() const noexcept;

private:
    void* do_allocate(std::size_t bytes, std::size_t alignment) override;
    void do_deallocate(void* pointer, std::size_t bytes, std::size_t alignment) override;
    [[nodiscard]] bool do_is_equal(
        const std::pmr::memory_resource& other) const noexcept override;

    IAllocator* m_allocator = nullptr;
};

}
