#pragma once

#include <cstdint>
#include <limits>

namespace spy::rhi
{

class Fence
{
public:
    virtual ~Fence() = default;

    Fence(const Fence&) = delete;
    Fence& operator=(const Fence&) = delete;

    [[nodiscard]] virtual bool isSignaled() const = 0;
    [[nodiscard]] virtual bool wait(
        std::uint64_t timeoutNanoseconds = std::numeric_limits<std::uint64_t>::max()) = 0;
    virtual void reset() = 0;

protected:
    Fence() = default;
};

}
