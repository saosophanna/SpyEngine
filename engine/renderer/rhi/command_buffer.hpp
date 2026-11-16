#pragma once

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

protected:
    CommandBuffer() = default;
};

}
