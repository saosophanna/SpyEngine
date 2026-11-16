#pragma once

#include "core/utilities/types/handle.hpp"

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

namespace spy {

template<typename T, typename Tag = void>
class SlotMap
{
    struct Slot
    {
        typename Handle<Tag>::Generation generation = Handle<Tag>::kInitialGeneration;
        std::uint32_t denseIndex = Handle<Tag>::kInvalidIndex;
        bool occupied = false;
    };

    struct Entry
    {
        std::uint32_t slotIndex = 0;
        T value;
    };

public:
    using Value = T;
    using HandleType = Handle<Tag>;

    SlotMap() = default;

    [[nodiscard]] std::size_t size() const noexcept { return m_dense.size(); }
    [[nodiscard]] bool empty() const noexcept { return m_dense.empty(); }

    HandleType insert(const T& value) { return emplace(value); }
    HandleType insert(T&& value) { return emplace(std::move(value)); }

    template<typename... Args>
    HandleType emplace(Args&&... args)
    {
        std::uint32_t slotIndex = 0;
        const bool reused = !m_free.empty();

        if (reused)
        {
            slotIndex = m_free.back();
        }
        else
        {
            slotIndex = static_cast<std::uint32_t>(m_slots.size());
            m_slots.push_back(Slot{});
        }

        m_dense.push_back(Entry{slotIndex, T(std::forward<Args>(args)...)});

        if (reused)
        {
            m_free.pop_back();
        }

        Slot& slot = m_slots[slotIndex];
        slot.occupied = true;
        slot.denseIndex = static_cast<std::uint32_t>(m_dense.size() - 1);

        return HandleType(slotIndex, slot.generation);
    }

    bool erase(HandleType handle)
    {
        if (!contains(handle))
        {
            return false;
        }

        Slot& slot = m_slots[handle.index()];
        const std::uint32_t denseIndex = slot.denseIndex;
        const std::uint32_t lastIndex = static_cast<std::uint32_t>(m_dense.size() - 1);

        if (denseIndex != lastIndex)
        {
            m_dense[denseIndex] = std::move(m_dense[lastIndex]);
            m_slots[m_dense[denseIndex].slotIndex].denseIndex = denseIndex;
        }

        m_dense.pop_back();

        slot.occupied = false;
        slot.denseIndex = HandleType::kInvalidIndex;

        ++slot.generation;

        if (slot.generation == 0)
        {
            slot.generation = HandleType::kInitialGeneration;
        }

        m_free.push_back(handle.index());
        return true;
    }

    [[nodiscard]] bool contains(HandleType handle) const noexcept
    {
        if (!handle.isValid() || handle.index() >= m_slots.size())
        {
            return false;
        }

        const Slot& slot = m_slots[handle.index()];
        return slot.occupied && slot.generation == handle.generation();
    }

    [[nodiscard]] T* tryGet(HandleType handle) noexcept
    {
        if (!contains(handle))
        {
            return nullptr;
        }

        return &m_dense[m_slots[handle.index()].denseIndex].value;
    }

    [[nodiscard]] const T* tryGet(HandleType handle) const noexcept
    {
        if (!contains(handle))
        {
            return nullptr;
        }

        return &m_dense[m_slots[handle.index()].denseIndex].value;
    }

    [[nodiscard]] T& get(HandleType handle) noexcept
    {
        return m_dense[m_slots[handle.index()].denseIndex].value;
    }

    [[nodiscard]] const T& get(HandleType handle) const noexcept
    {
        return m_dense[m_slots[handle.index()].denseIndex].value;
    }

    void clear() noexcept
    {
        m_dense.clear();
        m_slots.clear();
        m_free.clear();
    }

    class iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T*;
        using reference = T&;

        iterator() = default;
        explicit iterator(typename std::vector<Entry>::iterator internal) : m_internal(internal) {}

        reference operator*() const { return m_internal->value; }
        pointer operator->() const { return &m_internal->value; }

        iterator& operator++()
        {
            ++m_internal;
            return *this;
        }

        iterator operator++(int)
        {
            iterator previous = *this;
            ++m_internal;
            return previous;
        }

        friend bool operator==(const iterator&, const iterator&) = default;

    private:
        typename std::vector<Entry>::iterator m_internal{};
    };

    class const_iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = const T;
        using pointer = const T*;
        using reference = const T&;

        const_iterator() = default;
        explicit const_iterator(typename std::vector<Entry>::const_iterator internal)
            : m_internal(internal)
        {
        }

        reference operator*() const { return m_internal->value; }
        pointer operator->() const { return &m_internal->value; }

        const_iterator& operator++()
        {
            ++m_internal;
            return *this;
        }

        const_iterator operator++(int)
        {
            const_iterator previous = *this;
            ++m_internal;
            return previous;
        }

        friend bool operator==(const const_iterator&, const const_iterator&) = default;

    private:
        typename std::vector<Entry>::const_iterator m_internal{};
    };

    [[nodiscard]] iterator begin() noexcept { return iterator(m_dense.begin()); }
    [[nodiscard]] iterator end() noexcept { return iterator(m_dense.end()); }
    [[nodiscard]] const_iterator begin() const noexcept { return const_iterator(m_dense.begin()); }
    [[nodiscard]] const_iterator end() const noexcept { return const_iterator(m_dense.end()); }

private:
    std::vector<Slot> m_slots;
    std::vector<Entry> m_dense;
    std::vector<std::uint32_t> m_free;
};

}
