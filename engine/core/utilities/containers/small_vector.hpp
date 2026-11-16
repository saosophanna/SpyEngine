#pragma once

#include <cstddef>
#include <initializer_list>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace spy {

template<typename T, std::size_t InlineCapacity>
class SmallVector
{
public:
    using value_type = T;
    using iterator = T*;
    using const_iterator = const T*;

    SmallVector() noexcept = default;

    SmallVector(const SmallVector& other)
    {
        reserve(other.m_size);

        try
        {
            for (const T& value : other)
            {
                std::construct_at(data() + m_size, value);
                ++m_size;
            }
        }
        catch (...)
        {
            clear();
            throw;
        }
    }

    SmallVector(SmallVector&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        if (other.m_heap != nullptr)
        {
            m_heap = other.m_heap;
            m_heapCapacity = other.m_heapCapacity;
            m_size = other.m_size;

            other.m_heap = nullptr;
            other.m_heapCapacity = 0;
            other.m_size = 0;
            return;
        }

        for (T& value : other)
        {
            constructMovedFromElement(value);
        }

        other.clear();
    }

    SmallVector(std::initializer_list<T> values)
    {
        reserve(values.size());

        for (const T& value : values)
        {
            std::construct_at(data() + m_size, value);
            ++m_size;
        }
    }

    SmallVector& operator=(const SmallVector& other)
    {
        if (this != &other)
        {
            clear();
            reserve(other.m_size);

            for (const T& value : other)
            {
                std::construct_at(data() + m_size, value);
                ++m_size;
            }
        }

        return *this;
    }

    SmallVector& operator=(SmallVector&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        if (this != &other)
        {
            release();

            if (other.m_heap != nullptr)
            {
                m_heap = other.m_heap;
                m_heapCapacity = other.m_heapCapacity;
                m_size = other.m_size;

                other.m_heap = nullptr;
                other.m_heapCapacity = 0;
                other.m_size = 0;
            }
            else
            {
                for (T& value : other)
                {
                    constructMovedFromElement(value);
                }

                other.clear();
            }
        }

        return *this;
    }

    ~SmallVector() { release(); }

    [[nodiscard]] std::size_t size() const noexcept { return m_size; }
    [[nodiscard]] bool empty() const noexcept { return m_size == 0; }
    [[nodiscard]] std::size_t capacity() const noexcept
    {
        return m_heap != nullptr ? m_heapCapacity : InlineCapacity;
    }

    [[nodiscard]] T* data() noexcept
    {
        return m_heap != nullptr ? m_heap : inlineData();
    }

    [[nodiscard]] const T* data() const noexcept
    {
        return m_heap != nullptr ? m_heap : inlineData();
    }

    [[nodiscard]] iterator begin() noexcept { return data(); }
    [[nodiscard]] iterator end() noexcept { return data() + m_size; }
    [[nodiscard]] const_iterator begin() const noexcept { return data(); }
    [[nodiscard]] const_iterator end() const noexcept { return data() + m_size; }

    [[nodiscard]] T& operator[](std::size_t index) noexcept { return data()[index]; }
    [[nodiscard]] const T& operator[](std::size_t index) const noexcept { return data()[index]; }

    [[nodiscard]] T& front() noexcept { return data()[0]; }
    [[nodiscard]] const T& front() const noexcept { return data()[0]; }
    [[nodiscard]] T& back() noexcept { return data()[m_size - 1]; }
    [[nodiscard]] const T& back() const noexcept { return data()[m_size - 1]; }

    void reserve(std::size_t requested)
    {
        if (requested <= capacity())
        {
            return;
        }

        growTo(requested);
    }

    template<typename... Args>
    T& emplaceBack(Args&&... args)
    {
        if (m_size == capacity())
        {
            growTo(m_size + 1);
        }

        T* value = std::construct_at(data() + m_size, std::forward<Args>(args)...);
        ++m_size;
        return *value;
    }

    T& pushBack(const T& value) { return emplaceBack(value); }
    T& pushBack(T&& value) { return emplaceBack(std::move(value)); }

    void popBack() noexcept
    {
        if (m_size == 0)
        {
            return;
        }

        --m_size;
        std::destroy_at(data() + m_size);
    }

    void clear() noexcept
    {
        while (m_size > 0)
        {
            popBack();
        }
    }

private:
    void constructMovedFromElement(T& value)
    {
        try
        {
            std::construct_at(data() + m_size, std::move(value));
        }
        catch (...)
        {
            clear();
            throw;
        }

        ++m_size;
    }

    static constexpr std::size_t kMinHeapCapacity = 8;
    static constexpr std::size_t kInlineSize =
        sizeof(T) * InlineCapacity > 0 ? sizeof(T) * InlineCapacity : 1;

    T* inlineData() noexcept
    {
        return InlineCapacity == 0 ? nullptr : std::launder(reinterpret_cast<T*>(m_inline));
    }

    const T* inlineData() const noexcept
    {
        return InlineCapacity == 0 ? nullptr : std::launder(reinterpret_cast<const T*>(m_inline));
    }

    static T* allocate(std::size_t count)
    {
        std::allocator<T> allocator;
        return std::allocator_traits<std::allocator<T>>::allocate(allocator, count);
    }

    static void deallocate(T* pointer, std::size_t count) noexcept
    {
        std::allocator<T> allocator;
        std::allocator_traits<std::allocator<T>>::deallocate(allocator, pointer, count);
    }

    void growTo(std::size_t requested)
    {
        std::size_t newCapacity = capacity() > 0 ? capacity() : kMinHeapCapacity;

        while (newCapacity < requested)
        {
            newCapacity *= 2;
        }

        T* newData = allocate(newCapacity);
        std::size_t constructed = 0;

        try
        {
            for (; constructed < m_size; ++constructed)
            {
                std::construct_at(newData + constructed,
                                  std::move_if_noexcept(data()[constructed]));
            }
        }
        catch (...)
        {
            for (std::size_t index = 0; index < constructed; ++index)
            {
                std::destroy_at(newData + index);
            }

            deallocate(newData, newCapacity);
            throw;
        }

        const std::size_t retained = m_size;

        for (std::size_t index = 0; index < retained; ++index)
        {
            std::destroy_at(data() + index);
        }

        if (m_heap != nullptr)
        {
            deallocate(m_heap, m_heapCapacity);
        }

        m_heap = newData;
        m_heapCapacity = newCapacity;
        m_size = retained;
    }

    void release() noexcept
    {
        for (std::size_t index = 0; index < m_size; ++index)
        {
            std::destroy_at(data() + index);
        }

        if (m_heap != nullptr)
        {
            deallocate(m_heap, m_heapCapacity);
            m_heap = nullptr;
            m_heapCapacity = 0;
        }

        m_size = 0;
    }

    alignas(T) std::byte m_inline[kInlineSize];
    T* m_heap = nullptr;
    std::size_t m_heapCapacity = 0;
    std::size_t m_size = 0;
};

}
