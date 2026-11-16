#pragma once

#include <cstdint>

namespace spy::rhi {

class Buffer;
class Pipeline;
class Texture;

struct Color {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};

class CommandBuffer {
public:
    virtual ~CommandBuffer() = default;

    CommandBuffer(const CommandBuffer&)            = delete;
    CommandBuffer& operator=(const CommandBuffer&) = delete;

    virtual void begin() = 0;
    virtual void end()   = 0;

    virtual void beginRenderPass(const Color& clearColor) = 0;
    virtual void endRenderPass() = 0;

    virtual void bindPipeline(const Pipeline& pipeline) = 0;
    virtual void bindVertexBuffer(const Buffer& buffer) = 0;
    virtual void bindIndexBuffer(const Buffer& buffer) = 0;
    virtual void bindTexture(const Texture& texture, std::uint32_t slot = 0) = 0;

    virtual void draw(std::uint32_t vertexCount, std::uint32_t firstVertex = 0) = 0;
    virtual void drawIndexed(std::uint32_t indexCount, std::uint32_t firstIndex = 0) = 0;

protected:
    CommandBuffer() = default;
};

}
