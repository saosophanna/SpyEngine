#pragma once

#include <cstddef>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace spy {

template<typename T, std::size_t Capacity>
class FixedVector
{
public:
    using value_type = T;
    using iterator = T*;
    using const_iterator = const T*;

    FixedVector() noexcept = default;

    FixedVector(const FixedVector& other)
    {
        try
        {
            for (const T& value : other)
            {
                pushBackUnchecked(value);
            }
        }
        catch (...)
        {
            clear();
            throw;
        }
    }

    FixedVector(FixedVector&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        constructMovedFrom(other);
        other.clear();
    }

    FixedVector& operator=(const FixedVector& other)
    {
        if (this != &other)
        {
            clear();

            for (const T& value : other)
            {
                pushBackUnchecked(value);
            }
        }

        return *this;
    }

    FixedVector& operator=(FixedVector&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        if (this != &other)
        {
            clear();

            for (T& value : other)
            {
                pushBackUnchecked(std::move(value));
            }

            other.clear();
        }

        return *this;
    }

    ~FixedVector() { clear(); }

    [[nodiscard]] static constexpr std::size_t capacity() noexcept { return Capacity; }
    [[nodiscard]] std::size_t size() const noexcept { return m_size; }
    [[nodiscard]] bool empty() const noexcept { return m_size == 0; }
    [[nodiscard]] bool full() const noexcept { return m_size == Capacity; }

    [[nodiscard]] T* data() noexcept
    {
        return std::launder(reinterpret_cast<T*>(m_storage));
    }

    [[nodiscard]] const T* data() const noexcept
    {
        return std::launder(reinterpret_cast<const T*>(m_storage));
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

    template<typename... Args>
    T* emplaceBack(Args&&... args)
    {
        if (full())
        {
            return nullptr;
        }

        T* value = std::construct_at(data() + m_size, std::forward<Args>(args)...);
        ++m_size;
        return value;
    }

    T* pushBack(const T& value) { return emplaceBack(value); }
    T* pushBack(T&& value) { return emplaceBack(std::move(value)); }

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
    void constructMovedFrom(FixedVector& other)
    {
        try
        {
            for (T& value : other)
            {
                pushBackUnchecked(std::move(value));
            }
        }
        catch (...)
        {
            clear();
            throw;
        }
    }

    void pushBackUnchecked(const T& value)
    {
        if (m_size == Capacity)
        {
            return;
        }

        std::construct_at(data() + m_size, value);
        ++m_size;
    }

    void pushBackUnchecked(T&& value)
    {
        if (m_size == Capacity)
        {
            return;
        }

        std::construct_at(data() + m_size, std::move(value));
        ++m_size;
    }

    static constexpr std::size_t kStorageSize = sizeof(T) * Capacity > 0 ? sizeof(T) * Capacity : 1;

    alignas(T) std::byte m_storage[kStorageSize];
    std::size_t m_size = 0;
};

}
