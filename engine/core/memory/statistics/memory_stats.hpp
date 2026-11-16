#pragma once

#include <cstddef>
#include <cstdint>

namespace spy::memory {

struct MemoryStats
{
    std::size_t totalMemory = 0;
    std::size_t usedMemory = 0;
    std::size_t peakMemory = 0;

    std::size_t allocationCount = 0;
    std::size_t deallocationCount = 0;
    std::size_t failedAllocationCount = 0;

    std::uint32_t allocatorCount = 0;
};

}
