#pragma once

#include "core/utilities/types/types.hpp"

#include <atomic>
#include <cstddef>
#include <functional>
#include <limits>

namespace spy {

template<typename Tag>
class Id
{
public:
    using Value = std::uint64_t;

    static constexpr Value kInvalidValue = 0;

    constexpr Id() noexcept = default;
    constexpr explicit Id(Value value) noexcept : m_value(value) {}

    [[nodiscard]] constexpr Value value() const noexcept { return m_value; }
    [[nodiscard]] constexpr Value hashValue() const noexcept { return m_value; }
    [[nodiscard]] constexpr bool isValid() const noexcept { return m_value != kInvalidValue; }
    constexpr explicit operator bool() const noexcept { return isValid(); }

    friend constexpr bool operator==(Id lhs, Id rhs) noexcept = default;

private:
    Value m_value = kInvalidValue;
};

template<typename Tag>
struct IdHash
{
    [[nodiscard]] std::size_t operator()(Id<Tag> id) const noexcept
    {
        return static_cast<std::size_t>(id.value());
    }
};

template<typename Tag>
class IdGenerator
{
public:
    using IdType = Id<Tag>;
    using Value = typename IdType::Value;

    static constexpr Value kFirstValue = 1;

    constexpr IdGenerator() noexcept = default;

    [[nodiscard]] IdType next() noexcept
    {
        Value current = m_counter.load(std::memory_order_relaxed);

        while (current >= kFirstValue && current < std::numeric_limits<Value>::max())
        {
            if (m_counter.compare_exchange_weak(
                    current, current + 1, std::memory_order_relaxed, std::memory_order_relaxed))
            {
                return IdType(current);
            }
        }

        return IdType();
    }

    [[nodiscard]] Value lastIssued() const noexcept
    {
        return m_counter.load(std::memory_order_relaxed) - 1;
    }

private:
    std::atomic<Value> m_counter{kFirstValue};
};

}

namespace std {

template<typename Tag>
struct hash<spy::Id<Tag>>
{
    [[nodiscard]] size_t operator()(spy::Id<Tag> id) const noexcept
    {
        return static_cast<size_t>(id.value());
    }
};

}
