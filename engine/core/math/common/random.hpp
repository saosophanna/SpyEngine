#pragma once

#include <cstddef>
#include <cstdint>

#include "core/utilities/types/types.hpp"

namespace spy::math {

class Random
{
public:
    explicit Random(uint64 seedValue = 0x853c49e6748fea9bULL) : m_state(seedValue) {}

    void seed(uint64 seedValue) noexcept { m_state = seedValue; }

    [[nodiscard]] uint64 nextUint64() noexcept
    {
        m_state += 0x9e3779b97f4a7c15ULL;
        uint64 value = m_state;
        value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
        value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
        return value ^ (value >> 31);
    }

    [[nodiscard]] uint32 nextUint32() noexcept
    {
        return static_cast<uint32>(nextUint64() >> 32);
    }

    [[nodiscard]] float32 nextFloat() noexcept
    {
        return static_cast<float32>(nextUint64() >> 40) * (1.0f / 16777216.0f);
    }

    [[nodiscard]] float32 rangeFloat(float32 low, float32 high) noexcept
    {
        return low + (high - low) * nextFloat();
    }

    [[nodiscard]] uint32 rangeUint(uint32 low, uint32 high) noexcept
    {
        if (low >= high)
        {
            return low;
        }

        const uint64 span = static_cast<uint64>(high) - low + 1ULL;
        return low + static_cast<uint32>(nextUint64() % span);
    }

    [[nodiscard]] usize index(usize count) noexcept
    {
        if (count == 0)
        {
            return 0;
        }

        return static_cast<usize>(nextUint64() % count);
    }

    [[nodiscard]] bool chance(float32 probability) noexcept
    {
        return nextFloat() < probability;
    }

private:
    uint64 m_state;
};

}
