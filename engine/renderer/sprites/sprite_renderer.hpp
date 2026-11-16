#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "core/math/matrix/mat4.hpp"
#include "core/math/vector/vec2.hpp"
#include "core/math/vector/vec4.hpp"
#include "rhi/buffer.hpp"
#include "rhi/pipeline.hpp"
#include "rhi/render_context.hpp"
#include "rhi/texture.hpp"

namespace spy::rhi
{
class Device;
}

namespace spy::renderer
{

struct SpriteQuad
{
    math::Vec2 position{};
    math::Vec2 size{};
    math::Vec4 uv{0.0f, 0.0f, 1.0f, 1.0f};
    math::Vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
    float rotation = 0.0f;
};

class SpriteRenderer
{
public:
    SpriteRenderer() = default;
    ~SpriteRenderer();

    SpriteRenderer(const SpriteRenderer&) = delete;
    SpriteRenderer& operator=(const SpriteRenderer&) = delete;

    [[nodiscard]] bool initialize(rhi::Device& device,
                                  rhi::RenderContext& context,
                                  std::string_view vertexShaderPath,
                                  std::string_view fragmentShaderPath);
    void shutdown();

    [[nodiscard]] bool createTexture(std::string_view name,
                                     std::uint32_t width,
                                     std::uint32_t height,
                                     const void* rgbaPixels);
    [[nodiscard]] bool hasTexture(std::string_view name) const;

    void setProjection(const math::Mat4& projection);
    void setClip(float x, float y, float width, float height);
    void clearClip();
    void begin();
    void draw(std::string_view texture, const SpriteQuad& quad);
    void end(rhi::CommandBuffer& commandBuffer);

private:
    static constexpr std::uint32_t kMaxQuads = 16384;

    struct SpriteVertex
    {
        math::Vec2 position;
        math::Vec2 texCoord;
        math::Vec4 color;
    };

    struct Batch
    {
        rhi::Texture* texture = nullptr;
        std::uint32_t firstVertex = 0;
        std::uint32_t vertexCount = 0;
        float clipX = 0.0f;
        float clipY = 0.0f;
        float clipWidth = 16384.0f;
        float clipHeight = 16384.0f;
        bool clipped = false;
    };

    void appendVertex(const math::Vec2& position,
                      float u,
                      float v,
                      const math::Vec4& color);
    void closeBatch();

    rhi::Device* m_device = nullptr;
    rhi::RenderContext* m_context = nullptr;
    std::unique_ptr<rhi::Pipeline> m_pipeline;
    std::unique_ptr<rhi::Buffer> m_vertexBuffer;
    std::unordered_map<std::string, std::unique_ptr<rhi::Texture>> m_textures;
    std::vector<SpriteVertex> m_vertices;
    std::vector<Batch> m_batches;
    math::Mat4 m_projection{};
    rhi::Texture* m_currentTexture = nullptr;
    std::uint32_t m_currentBatchStart = 0;
    float m_clipX = 0.0f;
    float m_clipY = 0.0f;
    float m_clipWidth = 16384.0f;
    float m_clipHeight = 16384.0f;
    bool m_clipActive = false;
    bool m_frameOpen = false;
    bool m_ready = false;
};

}
