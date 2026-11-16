#pragma once

#include <cstddef>
#include <cstdint>

namespace spy::math {

class Random
{
public:
    explicit Random(std::uint64_t seedValue = 0x853c49e6748fea9bULL) : m_state(seedValue) {}

    void seed(std::uint64_t seedValue) noexcept { m_state = seedValue; }

    [[nodiscard]] std::uint64_t nextUint64() noexcept
    {
        m_state += 0x9e3779b97f4a7c15ULL;
        std::uint64_t value = m_state;
        value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
        value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
        return value ^ (value >> 31);
    }

    [[nodiscard]] std::uint32_t nextUint32() noexcept
    {
        return static_cast<std::uint32_t>(nextUint64() >> 32);
    }

    [[nodiscard]] float nextFloat() noexcept
    {
        return static_cast<float>(nextUint64() >> 40) * (1.0f / 16777216.0f);
    }

    [[nodiscard]] float rangeFloat(float low, float high) noexcept
    {
        return low + (high - low) * nextFloat();
    }

    [[nodiscard]] std::uint32_t rangeUint(std::uint32_t low, std::uint32_t high) noexcept
    {
        if (low >= high)
        {
            return low;
        }

        const std::uint64_t span = static_cast<std::uint64_t>(high) - low + 1ULL;
        return low + static_cast<std::uint32_t>(nextUint64() % span);
    }

    [[nodiscard]] std::size_t index(std::size_t count) noexcept
    {
        if (count == 0)
        {
            return 0;
        }

        return static_cast<std::size_t>(nextUint64() % count);
    }

    [[nodiscard]] bool chance(float probability) noexcept
    {
        return nextFloat() < probability;
    }

private:
    std::uint64_t m_state;
};

}
