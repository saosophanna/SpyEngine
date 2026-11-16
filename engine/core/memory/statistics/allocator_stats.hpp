#pragma once

#include <cstddef>

#include "core/utilities/types/types.hpp"

namespace spy::memory {

struct AllocatorStats
{
    usize totalMemory = 0;
    usize usedMemory = 0;
    usize peakMemory = 0;

    usize allocationCount = 0;
    usize deallocationCount = 0;
    usize failedAllocationCount = 0;
};

}
