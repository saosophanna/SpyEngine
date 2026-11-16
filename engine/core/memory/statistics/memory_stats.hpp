#pragma once

#include <cstddef>
#include <cstdint>

#include "core/utilities/types/types.hpp"

namespace spy::memory {

struct MemoryStats
{
    usize totalMemory = 0;
    usize usedMemory = 0;
    usize peakMemory = 0;

    usize allocationCount = 0;
    usize deallocationCount = 0;
    usize failedAllocationCount = 0;

    uint32 allocatorCount = 0;
};

}
