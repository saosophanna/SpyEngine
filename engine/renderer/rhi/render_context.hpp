#pragma once

#include <cstdint>

#include "rhi/command_buffer.hpp"
#include "rhi/native_window.hpp"
#include "rhi/pipeline.hpp"
#include "rhi/swapchain.hpp"

namespace spy::rhi
{

class RenderContext
{
public:
    virtual ~RenderContext() = default;

    RenderContext(const RenderContext&) = delete;
    RenderContext& operator=(const RenderContext&) = delete;

    [[nodiscard]] virtual bool initialize(const NativeWindow& window,
                                          std::uint32_t width,
                                          std::uint32_t height,
                                          bool vsync) = 0;
    virtual void shutdown() = 0;

    [[nodiscard]] virtual bool resize(std::uint32_t width, std::uint32_t height) = 0;
    [[nodiscard]] virtual bool beginFrame() = 0;
    [[nodiscard]] virtual bool endFrame() = 0;

    virtual void waitIdle() = 0;

    [[nodiscard]] virtual std::unique_ptr<Pipeline> createPipeline(
        const PipelineDesc& desc) = 0;

    [[nodiscard]] virtual std::uint32_t width() const = 0;
    [[nodiscard]] virtual std::uint32_t height() const = 0;
    [[nodiscard]] virtual std::uint32_t imageCount() const = 0;
    [[nodiscard]] virtual std::uint32_t currentFrameIndex() const = 0;

    [[nodiscard]] virtual CommandBuffer& currentCommandBuffer() = 0;
    [[nodiscard]] virtual Swapchain& swapchain() = 0;

protected:
    RenderContext() = default;
};

}
