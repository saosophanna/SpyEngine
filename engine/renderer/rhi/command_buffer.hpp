#pragma once

#include <cstddef>
#include <cstdint>

#include "rhi/buffer.hpp"
#include "rhi/pipeline.hpp"
#include "rhi/texture.hpp"

namespace spy::rhi
{

struct Color
{
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};

class CommandBuffer
{
public:
    virtual ~CommandBuffer() = default;

    CommandBuffer(const CommandBuffer&) = delete;
    CommandBuffer& operator=(const CommandBuffer&) = delete;

    [[nodiscard]] virtual bool begin() = 0;
    [[nodiscard]] virtual bool end() = 0;

    virtual void beginRenderPass(const Color& clearColor) = 0;
    virtual void endRenderPass() = 0;

    virtual void setScissor(std::uint32_t x,
                            std::uint32_t y,
                            std::uint32_t width,
                            std::uint32_t height) = 0;

    virtual void bindPipeline(Pipeline& pipeline) = 0;
    virtual void bindVertexBuffer(Buffer& buffer) = 0;
    virtual void bindTexture(Texture& texture, std::uint32_t slot = 0) = 0;
    virtual void pushConstants(const void* data,
                               std::uint32_t size,
                               std::uint32_t offset = 0) = 0;
    virtual void draw(std::uint32_t vertexCount, std::uint32_t firstVertex = 0) = 0;

protected:
    CommandBuffer() = default;
};

}
