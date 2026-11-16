#include "core/memory/adapters/allocator_resource.hpp"

#include <new>

namespace spy::memory {

AllocatorResource::AllocatorResource(IAllocator& allocator) noexcept
    : m_allocator(&allocator)
{
}

IAllocator* AllocatorResource::allocator() const noexcept
{
    return m_allocator;
}

void* AllocatorResource::do_allocate(std::size_t bytes, std::size_t alignment)
{
    void* memory = m_allocator->allocate(bytes, alignment);

    if (memory == nullptr)
    {
        throw std::bad_alloc();
    }

    return memory;
}

void AllocatorResource::do_deallocate(void* pointer, std::size_t bytes, std::size_t alignment)
{
    (void)bytes;
    (void)alignment;

    m_allocator->deallocate(pointer);
}

bool AllocatorResource::do_is_equal(const std::pmr::memory_resource& other) const noexcept
{
    if (this == &other)
    {
        return true;
    }

    const auto* resource = dynamic_cast<const AllocatorResource*>(&other);

    return resource != nullptr && resource->m_allocator == m_allocator;
}

}
