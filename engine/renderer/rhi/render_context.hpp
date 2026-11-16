#pragma once

#include <cstdint>

namespace spy::rhi {

class CommandBuffer;

class RenderContext {
public:
    virtual ~RenderContext() = default;

    RenderContext(const RenderContext&)            = delete;
    RenderContext& operator=(const RenderContext&) = delete;

    virtual bool createSurface(void* nativeWindowHandle) = 0;
    virtual bool resize(std::uint32_t width, std::uint32_t height) = 0;

    virtual bool beginFrame() = 0;
    virtual bool endFrame()   = 0;

    virtual void waitIdle() = 0;

    [[nodiscard]] virtual std::uint32_t width()      const = 0;
    [[nodiscard]] virtual std::uint32_t height()     const = 0;
    [[nodiscard]] virtual std::uint32_t imageCount() const = 0;

    [[nodiscard]] virtual CommandBuffer& currentCommandBuffer() = 0;

protected:
    RenderContext() = default;
};

}
