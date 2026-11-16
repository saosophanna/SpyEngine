#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <utility>

#include "core/memory/memory.hpp"

namespace {

int g_failures = 0;

void check(bool condition, const char* expression, int line)
{
    if (!condition)
    {
        std::fprintf(stderr, "FAILED: %s (line %d)\n", expression, line);
        ++g_failures;
    }
}

std::uintptr_t addressOf(const void* pointer)
{
    return reinterpret_cast<std::uintptr_t>(pointer);
}

}

#define SPY_CHECK(expression) check((expression), #expression, __LINE__)

int main()
{
    using namespace spy::memory;

    SPY_CHECK(isPowerOfTwo(1));
    SPY_CHECK(isPowerOfTwo(64));
    SPY_CHECK(!isPowerOfTwo(0));
    SPY_CHECK(!isPowerOfTwo(3));

    SPY_CHECK(alignUp(0, 16) == 0);
    SPY_CHECK(alignUp(1, 16) == 16);
    SPY_CHECK(alignUp(16, 16) == 16);
    SPY_CHECK(alignUp(17, 32) == 32);

    SPY_CHECK(alignmentPadding(0, 16) == 0);
    SPY_CHECK(alignmentPadding(1, 16) == 15);
    SPY_CHECK(alignmentPadding(16, 16) == 0);
    SPY_CHECK(alignmentPadding(17, 32) == 15);

    void* systemBlock = allocateSystem(64, 64);
    SPY_CHECK(systemBlock != nullptr);
    SPY_CHECK(isAligned(systemBlock, 64));
    std::memset(systemBlock, 0xAB, 64);
    deallocateSystem(systemBlock);
    deallocateSystem(nullptr);
    SPY_CHECK(allocateSystem(0, 8) == nullptr);
    SPY_CHECK(allocateSystem(8, 3) == nullptr);

    {
        LinearAllocator linear(1024);

        SPY_CHECK(linear.capacity() == 1024);
        SPY_CHECK(linear.totalMemory() == 1024);
        SPY_CHECK(linear.usedMemory() == 0);
        SPY_CHECK(linear.data() != nullptr);

        void* first = linear.allocate(16, 8);
        void* second = linear.allocate(64, 64);

        SPY_CHECK(first != nullptr);
        SPY_CHECK(second != nullptr);
        SPY_CHECK(isAligned(first, 8));
        SPY_CHECK(isAligned(second, 64));
        SPY_CHECK(addressOf(second) > addressOf(first));
        SPY_CHECK(linear.usedMemory() <= linear.capacity());
        SPY_CHECK(linear.stats().allocationCount == 2);
        SPY_CHECK(linear.stats().failedAllocationCount == 0);
        SPY_CHECK(linear.stats().peakMemory == linear.usedMemory());

        SPY_CHECK(linear.allocate(0, 8) == nullptr);
        SPY_CHECK(linear.allocate(8, 3) == nullptr);
        SPY_CHECK(linear.allocate(4096, 8) == nullptr);
        SPY_CHECK(linear.stats().failedAllocationCount == 3);

        linear.deallocate(first);
        SPY_CHECK(linear.usedMemory() == linear.stats().peakMemory);
        SPY_CHECK(!linear.capabilities().individualDeallocation);
        SPY_CHECK(linear.capabilities().reset);

        linear.reset();
        SPY_CHECK(linear.usedMemory() == 0);
        SPY_CHECK(linear.offset() == 0);
        SPY_CHECK(linear.allocate(16, 8) == first);
    }

    {
        alignas(64) std::byte externalBuffer[256];
        LinearAllocator external(externalBuffer, sizeof(externalBuffer));

        SPY_CHECK(external.totalMemory() == 256);

        void* block = external.allocate(8, 8);
        SPY_CHECK(block != nullptr);
        SPY_CHECK(addressOf(block) >= addressOf(externalBuffer));
        SPY_CHECK(addressOf(block) < addressOf(externalBuffer) + sizeof(externalBuffer));
    }

    {
        ArenaAllocator arena(64);

        SPY_CHECK(arena.blockSize() == 64);
        SPY_CHECK(arena.blockCount() == 0);
        SPY_CHECK(arena.usedMemory() == 0);

        void* first = arena.allocate(32, 8);
        void* second = arena.allocate(32, 8);

        SPY_CHECK(first != nullptr);
        SPY_CHECK(second != nullptr);
        SPY_CHECK(first != second);
        SPY_CHECK(isAligned(first, 8));
        SPY_CHECK(arena.blockCount() == 1);

        void* third = arena.allocate(32, 8);
        SPY_CHECK(third != nullptr);
        SPY_CHECK(arena.blockCount() == 2);

        void* aligned = arena.allocate(48, 64);
        SPY_CHECK(aligned != nullptr);
        SPY_CHECK(isAligned(aligned, 64));
        SPY_CHECK(arena.blockCount() == 3);

        SPY_CHECK(arena.allocate(0, 8) == nullptr);
        SPY_CHECK(arena.allocate(8, 3) == nullptr);
        SPY_CHECK(arena.allocate(static_cast<std::size_t>(-1), 8) == nullptr);
        SPY_CHECK(arena.stats().failedAllocationCount == 3);

        const std::size_t blocks = arena.blockCount();
        arena.reset();
        SPY_CHECK(arena.usedMemory() == 0);
        SPY_CHECK(arena.blockCount() == blocks);

        void* reused = arena.allocate(16, 8);
        SPY_CHECK(reused != nullptr);
        SPY_CHECK(arena.blockCount() == blocks);

        arena.release();
        SPY_CHECK(arena.blockCount() == 0);
        SPY_CHECK(arena.totalMemory() == 0);
        SPY_CHECK(arena.usedMemory() == 0);
        SPY_CHECK(arena.allocate(16, 8) != nullptr);
        SPY_CHECK(arena.blockCount() == 1);
        SPY_CHECK(arena.capabilities().growth);
    }

    {
        LinearAllocator backing(4096);
        ArenaAllocator arena(128, &backing);

        SPY_CHECK(arena.allocate(64, 8) != nullptr);
        SPY_CHECK(backing.usedMemory() > 0);
    }

    {
        StackAllocator stack(512);

        SPY_CHECK(stack.capacity() == 512);
        SPY_CHECK(stack.marker() == 0);

        void* first = stack.allocate(32, 8);
        SPY_CHECK(first != nullptr);

        const StackMarker marker = stack.marker();
        SPY_CHECK(marker >= 32);

        void* second = stack.allocate(64, 16);
        SPY_CHECK(second != nullptr);
        SPY_CHECK(isAligned(second, 16));

        stack.freeToMarker(marker);
        SPY_CHECK(stack.marker() == marker);

        void* third = stack.allocate(16, 8);
        SPY_CHECK(third != nullptr);
        SPY_CHECK(addressOf(third) <= addressOf(second));

        const StackMarker beforeScope = stack.marker();
        {
            ScopedStackAllocation scope(stack);
            SPY_CHECK(scope.marker() == beforeScope);
            SPY_CHECK(stack.allocate(32, 8) != nullptr);
            SPY_CHECK(stack.marker() > beforeScope);
        }
        SPY_CHECK(stack.marker() == beforeScope);

        {
            ScopedStackAllocation outer(stack);
            SPY_CHECK(stack.allocate(16, 8) != nullptr);
            ScopedStackAllocation moved(std::move(outer));
            SPY_CHECK(moved.marker() == beforeScope);
        }
        SPY_CHECK(stack.marker() == beforeScope);

        const StackMarker beforeFree = stack.marker();
        void* last = stack.allocate(24, 8);
        SPY_CHECK(last != nullptr);
        const StackMarker afterAllocate = stack.marker();
        stack.deallocate(last);
        SPY_CHECK(stack.marker() < afterAllocate);
        SPY_CHECK(stack.marker() >= beforeFree);
        SPY_CHECK(stack.stats().deallocationCount == 1);
        stack.deallocate(nullptr);

        const StackMarker current = stack.marker();
        stack.freeToMarker(current + 1000);
        SPY_CHECK(stack.marker() == current);

        SPY_CHECK(stack.allocate(0, 8) == nullptr);
        SPY_CHECK(stack.allocate(8, 3) == nullptr);
        SPY_CHECK(stack.allocate(4096, 8) == nullptr);
        SPY_CHECK(stack.capabilities().stackRewind);
    }

    {
        PoolAllocator pool(sizeof(std::uint64_t), alignof(std::uint64_t), 8);

        SPY_CHECK(pool.capacity() == 8);
        SPY_CHECK(pool.freeCount() == 8);
        SPY_CHECK(pool.objectSize() == sizeof(std::uint64_t));
        SPY_CHECK(pool.objectAlignment() == alignof(std::uint64_t));
        SPY_CHECK(pool.stride() >= sizeof(std::uint64_t));
        SPY_CHECK(pool.totalMemory() == pool.stride() * 8);

        std::array<void*, 8> blocks{};

        for (std::size_t index = 0; index < blocks.size(); ++index)
        {
            blocks[index] = pool.allocate();
            SPY_CHECK(blocks[index] != nullptr);
            SPY_CHECK(isAligned(blocks[index], alignof(std::uint64_t)));
        }

        SPY_CHECK(pool.freeCount() == 0);
        SPY_CHECK(pool.allocate() == nullptr);
        SPY_CHECK(pool.stats().failedAllocationCount == 1);
        SPY_CHECK(pool.usedMemory() == pool.stride() * pool.capacity());

        for (std::size_t index = 0; index < blocks.size(); ++index)
        {
            for (std::size_t other = index + 1; other < blocks.size(); ++other)
            {
                SPY_CHECK(blocks[index] != blocks[other]);
            }
        }

        pool.deallocate(blocks[0]);
        SPY_CHECK(pool.freeCount() == 1);
        pool.deallocate(blocks[0]);
        SPY_CHECK(pool.freeCount() == 1);
        pool.deallocate(nullptr);
        SPY_CHECK(pool.freeCount() == 1);

        void* reused = pool.allocate();
        SPY_CHECK(reused == blocks[0]);
        SPY_CHECK(pool.freeCount() == 0);

        pool.deallocate(blocks[1]);
        SPY_CHECK(pool.allocate(1, 8) == blocks[1]);
        pool.deallocate(blocks[1]);

        SPY_CHECK(pool.allocate(1, 3) == nullptr);
        SPY_CHECK(pool.allocate(1, 16) == nullptr);
        SPY_CHECK(pool.allocate(sizeof(std::uint64_t) + 1, 8) == nullptr);

        std::uint32_t foreign = 0;
        pool.deallocate(&foreign);
        SPY_CHECK(pool.freeCount() == 1);

        pool.reset();
        SPY_CHECK(pool.freeCount() == 8);
        SPY_CHECK(pool.usedMemory() == 0);
        SPY_CHECK(pool.allocate() != nullptr);
        SPY_CHECK(pool.capabilities().individualDeallocation);
    }

    {
        PoolAllocator normalized(1, 1, 2);

        SPY_CHECK(normalized.objectSize() == 1);
        SPY_CHECK(normalized.objectAlignment() == alignof(void*));
        SPY_CHECK(normalized.stride() >= sizeof(void*));
        SPY_CHECK(normalized.allocate() != nullptr);
        SPY_CHECK(normalized.allocate() != nullptr);
        SPY_CHECK(normalized.allocate() == nullptr);
    }

    {
        FreeListAllocator freeList(1024);

        SPY_CHECK(freeList.capacity() == 1024);
        SPY_CHECK(freeList.usedMemory() == 0);
        SPY_CHECK(freeList.freeMemory() == freeList.capacity());
        SPY_CHECK(freeList.largestFreeBlock() == freeList.capacity());

        void* first = freeList.allocate(64, 8);
        void* second = freeList.allocate(128, 32);
        void* third = freeList.allocate(256, 16);

        SPY_CHECK(first != nullptr);
        SPY_CHECK(second != nullptr);
        SPY_CHECK(third != nullptr);
        SPY_CHECK(isAligned(first, 8));
        SPY_CHECK(isAligned(second, 32));
        SPY_CHECK(isAligned(third, 16));
        SPY_CHECK(addressOf(first) < addressOf(second));
        SPY_CHECK(addressOf(second) < addressOf(third));
        SPY_CHECK(freeList.stats().allocationCount == 3);
        SPY_CHECK(freeList.usedMemory() + freeList.freeMemory() == freeList.capacity());

        std::memset(first, 0x11, 64);
        std::memset(second, 0x22, 128);
        std::memset(third, 0x33, 256);

        SPY_CHECK(static_cast<unsigned char*>(first)[0] == 0x11);
        SPY_CHECK(static_cast<unsigned char*>(first)[63] == 0x11);
        SPY_CHECK(static_cast<unsigned char*>(second)[0] == 0x22);
        SPY_CHECK(static_cast<unsigned char*>(second)[127] == 0x22);
        SPY_CHECK(static_cast<unsigned char*>(third)[0] == 0x33);
        SPY_CHECK(static_cast<unsigned char*>(third)[255] == 0x33);

        freeList.deallocate(second);
        SPY_CHECK(freeList.stats().deallocationCount == 1);
        SPY_CHECK(freeList.freeMemory() > 0);
        SPY_CHECK(freeList.usedMemory() + freeList.freeMemory() == freeList.capacity());

        freeList.deallocate(second);
        SPY_CHECK(freeList.stats().deallocationCount == 1);

        freeList.deallocate(first);
        freeList.deallocate(third);

        SPY_CHECK(freeList.stats().deallocationCount == 3);
        SPY_CHECK(freeList.usedMemory() == 0);
        SPY_CHECK(freeList.freeMemory() == freeList.capacity());
        SPY_CHECK(freeList.largestFreeBlock() == freeList.capacity());

        std::uint32_t foreign = 0;
        freeList.deallocate(&foreign);
        SPY_CHECK(freeList.stats().deallocationCount == 3);

        void* reused = freeList.allocate(512, 64);
        SPY_CHECK(reused != nullptr);
        SPY_CHECK(isAligned(reused, 64));
        SPY_CHECK(freeList.usedMemory() + freeList.freeMemory() == freeList.capacity());

        freeList.reset();
        SPY_CHECK(freeList.usedMemory() == 0);
        SPY_CHECK(freeList.freeMemory() == freeList.capacity());
        SPY_CHECK(freeList.largestFreeBlock() == freeList.capacity());

        SPY_CHECK(freeList.allocate(0, 8) == nullptr);
        SPY_CHECK(freeList.allocate(8, 3) == nullptr);
        SPY_CHECK(freeList.allocate(freeList.capacity(), 8) == nullptr);
        SPY_CHECK(freeList.capabilities().individualDeallocation);
    }

    {
        FreeListAllocator tiny(16);
        SPY_CHECK(tiny.capacity() == 0);
        SPY_CHECK(tiny.allocate(8, 8) == nullptr);
    }

    {
        constexpr std::size_t capacity = 256;
        alignas(8) std::array<std::byte, capacity + 8> guard{};
        LinearAllocator backing(guard.data(), guard.size());
        FreeListAllocator freeList(capacity, &backing);
        const std::uintptr_t begin = addressOf(guard.data());
        std::size_t accepted = 0;

        for (std::size_t size = 1; size <= capacity; ++size)
        {
            freeList.reset();
            void* block = freeList.allocate(size, 8);

            if (block == nullptr)
            {
                continue;
            }

            ++accepted;
            SPY_CHECK(addressOf(block) - begin + size <= capacity);
            std::memset(block, 0x5A, size);
        }

        SPY_CHECK(accepted > 0);
    }

    {
        AllocationTracker tracker(4);

        std::uint64_t first = 0;
        std::uint64_t second = 0;
        std::uint64_t third = 0;

        SPY_CHECK(tracker.capacity() == 4);
        SPY_CHECK(tracker.track(&first, sizeof(first), alignof(std::uint64_t)));
        SPY_CHECK(tracker.track(&second, sizeof(second)));
        SPY_CHECK(tracker.liveCount() == 2);
        SPY_CHECK(tracker.liveBytes() == sizeof(first) + sizeof(second));
        SPY_CHECK(tracker.contains(&first));

        const AllocationRecord* record = tracker.find(&first);
        SPY_CHECK(record != nullptr);
        SPY_CHECK(record->size == sizeof(first));
        SPY_CHECK(record->alignment == alignof(std::uint64_t));
        SPY_CHECK(record->location.line() != 0);

        SPY_CHECK(tracker.release(&first));
        SPY_CHECK(!tracker.contains(&first));
        SPY_CHECK(tracker.liveCount() == 1);
        SPY_CHECK(tracker.liveBytes() == sizeof(second));

        SPY_CHECK(!tracker.release(&first));
        SPY_CHECK(tracker.invalidReleaseCount() == 1);
        SPY_CHECK(tracker.doubleFreeDetected());

        SPY_CHECK(tracker.track(&first, sizeof(first)));
        SPY_CHECK(tracker.release(&first));
        SPY_CHECK(tracker.doubleFreeDetected());

        SPY_CHECK(!tracker.release(&third));
        SPY_CHECK(tracker.invalidReleaseCount() == 2);

        SPY_CHECK(tracker.track(&third, sizeof(third)));
        SPY_CHECK(!tracker.track(&third, sizeof(third)));
        SPY_CHECK(tracker.overflowCount() == 1);
        SPY_CHECK(!tracker.track(nullptr, sizeof(third)));
        SPY_CHECK(tracker.overflowCount() == 2);
    }

    {
        AllocationTracker tracker(1);
        std::uint32_t first = 0;
        std::uint32_t second = 0;

        SPY_CHECK(tracker.track(&first, sizeof(first)));
        SPY_CHECK(!tracker.track(&second, sizeof(second)));
        SPY_CHECK(tracker.overflowCount() == 1);

        tracker.clear();
        SPY_CHECK(tracker.liveCount() == 0);
        SPY_CHECK(tracker.liveBytes() == 0);
        SPY_CHECK(tracker.overflowCount() == 0);
        SPY_CHECK(tracker.invalidReleaseCount() == 0);
        SPY_CHECK(!tracker.doubleFreeDetected());
    }

    {
        MemoryManager manager;
        LinearAllocator linearAllocator(256);
        StackAllocator stackAllocator(256);

        SPY_CHECK(!manager.registerAllocator(1, linearAllocator));
        SPY_CHECK(manager.initialize());
        SPY_CHECK(manager.registerAllocator(1, linearAllocator));
        SPY_CHECK(manager.registerAllocator(2, stackAllocator));
        SPY_CHECK(!manager.registerAllocator(1, stackAllocator));
        SPY_CHECK(manager.allocatorCount() == 2);
        SPY_CHECK(manager.allocator(1) == &linearAllocator);
        SPY_CHECK(manager.allocator(2) == &stackAllocator);
        SPY_CHECK(manager.allocator(3) == nullptr);

        SPY_CHECK(linearAllocator.allocate(32, 8) != nullptr);
        SPY_CHECK(stackAllocator.allocate(32, 8) != nullptr);

        const MemoryStats stats = manager.stats();
        SPY_CHECK(stats.allocatorCount == 2);
        SPY_CHECK(stats.usedMemory > 0);
        SPY_CHECK(stats.allocationCount == 2);
        SPY_CHECK(stats.totalMemory ==
                  linearAllocator.totalMemory() + stackAllocator.totalMemory());

        manager.resetAllocators();
        SPY_CHECK(linearAllocator.usedMemory() == 0);
        SPY_CHECK(stackAllocator.usedMemory() == 0);

        SPY_CHECK(manager.unregisterAllocator(1));
        SPY_CHECK(manager.allocatorCount() == 1);
        SPY_CHECK(manager.allocator(1) == nullptr);
        SPY_CHECK(manager.allocator(2) == &stackAllocator);
        SPY_CHECK(!manager.unregisterAllocator(1));

        manager.shutdown();
        SPY_CHECK(manager.allocatorCount() == 0);
        SPY_CHECK(!manager.registerAllocator(5, linearAllocator));
    }

    {
        MemoryManager manager;
        LinearAllocator shared(64);

        SPY_CHECK(manager.initialize());

        for (std::uint32_t id = 1; id <= 16u; ++id)
        {
            SPY_CHECK(manager.registerAllocator(id, shared));
        }

        SPY_CHECK(!manager.registerAllocator(17, shared));
        SPY_CHECK(manager.allocatorCount() == 16);
    }

    if (g_failures != 0)
    {
        std::fprintf(stderr, "%d checks failed\n", g_failures);
        return 1;
    }

    return 0;
}
