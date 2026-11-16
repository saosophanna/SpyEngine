#pragma once

#include <cstddef>
#include <cstdint>

namespace spy::rhi {

enum class BufferUsage : std::uint32_t {
    Vertex  = 1u << 0,
    Index   = 1u << 1,
    Uniform = 1u << 2,
    Storage = 1u << 3,
};

inline BufferUsage operator|(BufferUsage a, BufferUsage b) {
    return static_cast<BufferUsage>(static_cast<std::uint32_t>(a) |
                                    static_cast<std::uint32_t>(b));
}

enum class MemoryUsage {
    GpuOnly,
    CpuToGpu,
    GpuToCpu,
};

struct BufferDesc {
    std::size_t size   = 0;
    BufferUsage usage  = BufferUsage::Vertex;
    MemoryUsage memory = MemoryUsage::GpuOnly;
};

class Buffer {
public:
    virtual ~Buffer() = default;

    Buffer(const Buffer&)            = delete;
    Buffer& operator=(const Buffer&) = delete;

    [[nodiscard]] virtual std::size_t size() const = 0;

    virtual bool write(const void* data, std::size_t bytes, std::size_t offset = 0) = 0;

    virtual void* map() = 0;
    virtual void  unmap() = 0;

protected:
    Buffer() = default;
};

}
