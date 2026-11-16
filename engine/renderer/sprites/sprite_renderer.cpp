#include "sprites/sprite_renderer.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iterator>

#include "core/logging/log_macros.hpp"
#include "rhi/device.hpp"

namespace spy::renderer
{

namespace
{

bool readShaderFile(std::string_view path, std::vector<std::uint8_t>& data)
{
    std::ifstream file(std::string(path), std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        return false;
    }

    const std::streamsize size = file.tellg();

    if (size <= 0 || size % 4 != 0)
    {
        return false;
    }

    file.seekg(0, std::ios::beg);
    data.resize(static_cast<std::size_t>(size));

    return file.read(reinterpret_cast<char*>(data.data()), size).good();
}

}

SpriteRenderer::~SpriteRenderer()
{
    shutdown();
}

bool SpriteRenderer::initialize(rhi::Device& device,
                                rhi::RenderContext& context,
                                std::string_view vertexShaderPath,
                                std::string_view fragmentShaderPath)
{
    shutdown();

    m_device = &device;
    m_context = &context;

    std::vector<std::uint8_t> vertexShader;
    std::vector<std::uint8_t> fragmentShader;

    if (!readShaderFile(vertexShaderPath, vertexShader) ||
        !readShaderFile(fragmentShaderPath, fragmentShader))
    {
        SPY_LOG_ERROR("Sprite shader files could not be read");
        m_device = nullptr;
        m_context = nullptr;
        return false;
    }

    rhi::ShaderModule stages[2]{};
    stages[0].stage = rhi::ShaderStage::Vertex;
    stages[0].code = vertexShader.data();
    stages[0].size = vertexShader.size();
    stages[1].stage = rhi::ShaderStage::Fragment;
    stages[1].code = fragmentShader.data();
    stages[1].size = fragmentShader.size();

    const rhi::VertexAttribute attributes[3] = {
        {0, rhi::VertexFormat::Float2, 0},
        {1, rhi::VertexFormat::Float2, sizeof(float) * 2},
        {2, rhi::VertexFormat::Float4, sizeof(float) * 4},
    };

    rhi::PipelineDesc pipelineDesc{};
    pipelineDesc.stages = stages;
    pipelineDesc.stageCount = 2;
    pipelineDesc.attributes = attributes;
    pipelineDesc.attributeCount = 3;
    pipelineDesc.vertexStride = static_cast<std::uint32_t>(sizeof(SpriteVertex));
    pipelineDesc.topology = rhi::PrimitiveTopology::TriangleList;
    pipelineDesc.blend = rhi::BlendMode::Alpha;
    pipelineDesc.cull = rhi::CullMode::None;
    pipelineDesc.depth = rhi::DepthMode::Disabled;
    pipelineDesc.pushConstantSize = static_cast<std::uint32_t>(sizeof(math::Mat4));
    pipelineDesc.hasTexture = true;

    m_pipeline = context.createPipeline(pipelineDesc);

    if (m_pipeline == nullptr || !m_pipeline->isValid())
    {
        SPY_LOG_ERROR("Sprite pipeline creation failed");
        shutdown();
        return false;
    }

    rhi::BufferDesc bufferDesc{};
    bufferDesc.size = static_cast<std::size_t>(kMaxQuads) * 6u * sizeof(SpriteVertex);
    bufferDesc.usage = rhi::BufferUsage::Vertex;
    bufferDesc.memory = rhi::MemoryUsage::CpuToGpu;

    m_vertexBuffer = device.createBuffer(bufferDesc);

    if (m_vertexBuffer == nullptr)
    {
        SPY_LOG_ERROR("Sprite vertex buffer creation failed");
        shutdown();
        return false;
    }

    m_ready = true;
    return true;
}

void SpriteRenderer::shutdown()
{
    m_textures.clear();
    m_vertexBuffer.reset();
    m_pipeline.reset();
    m_vertices.clear();
    m_batches.clear();
    m_currentTexture = nullptr;
    m_currentBatchStart = 0;
    m_frameOpen = false;
    m_device = nullptr;
    m_context = nullptr;
    m_ready = false;
}

bool SpriteRenderer::createTexture(std::string_view name,
                                   std::uint32_t width,
                                   std::uint32_t height,
                                   const void* rgbaPixels)
{
    if (!m_ready || m_device == nullptr || width == 0 || height == 0)
    {
        return false;
    }

    rhi::TextureDesc desc{};
    desc.width = width;
    desc.height = height;
    desc.mipLevels = 1;
    desc.format = rhi::TextureFormat::RGBA8_SRGB;
    desc.filter = rhi::TextureFilter::Linear;
    desc.wrap = rhi::TextureWrap::ClampToEdge;
    desc.initialPixels = rgbaPixels;

    std::unique_ptr<rhi::Texture> texture = m_device->createTexture(desc);

    if (texture == nullptr)
    {
        SPY_LOG_ERROR("Texture creation failed");
        return false;
    }

    m_textures[std::string(name)] = std::move(texture);
    return true;
}

bool SpriteRenderer::hasTexture(std::string_view name) const
{
    return m_textures.find(std::string(name)) != m_textures.end();
}

void SpriteRenderer::setProjection(const math::Mat4& projection)
{
    m_projection = projection;
}

void SpriteRenderer::setClip(float x, float y, float width, float height)
{
    closeBatch();

    m_clipX = std::max(x, 0.0f);
    m_clipY = std::max(y, 0.0f);
    m_clipWidth = std::max(width, 0.0f);
    m_clipHeight = std::max(height, 0.0f);
    m_clipActive = true;
}

void SpriteRenderer::clearClip()
{
    closeBatch();

    m_clipX = 0.0f;
    m_clipY = 0.0f;
    m_clipWidth = 16384.0f;
    m_clipHeight = 16384.0f;
    m_clipActive = false;
}

void SpriteRenderer::begin()
{
    m_vertices.clear();
    m_batches.clear();
    m_currentTexture = nullptr;
    m_currentBatchStart = 0;
    m_clipX = 0.0f;
    m_clipY = 0.0f;
    m_clipWidth = 16384.0f;
    m_clipHeight = 16384.0f;
    m_clipActive = false;
    m_frameOpen = true;
}

void SpriteRenderer::draw(std::string_view texture, const SpriteQuad& quad)
{
    if (!m_frameOpen)
    {
        return;
    }

    const auto found = m_textures.find(std::string(texture));

    if (found == m_textures.end() || found->second == nullptr)
    {
        return;
    }

    if (m_vertices.size() + 6 > static_cast<std::size_t>(kMaxQuads) * 6)
    {
        return;
    }

    rhi::Texture* current = found->second.get();

    if (m_currentTexture != current)
    {
        closeBatch();
        m_currentTexture = current;
        m_currentBatchStart = static_cast<std::uint32_t>(m_vertices.size());
    }

    const float cosine = std::cos(quad.rotation);
    const float sine = std::sin(quad.rotation);
    const math::Vec2 half = quad.size * 0.5f;
    const math::Vec2 center = quad.position + half;

    const auto corner = [&](float localX, float localY)
    {
        return math::Vec2{center.x + localX * cosine - localY * sine,
                          center.y + localX * sine + localY * cosine};
    };

    const math::Vec2 topLeft = corner(-half.x, -half.y);
    const math::Vec2 topRight = corner(half.x, -half.y);
    const math::Vec2 bottomRight = corner(half.x, half.y);
    const math::Vec2 bottomLeft = corner(-half.x, half.y);

    appendVertex(topLeft, quad.uv.x, quad.uv.y, quad.color);
    appendVertex(topRight, quad.uv.z, quad.uv.y, quad.color);
    appendVertex(bottomRight, quad.uv.z, quad.uv.w, quad.color);

    appendVertex(topLeft, quad.uv.x, quad.uv.y, quad.color);
    appendVertex(bottomRight, quad.uv.z, quad.uv.w, quad.color);
    appendVertex(bottomLeft, quad.uv.x, quad.uv.w, quad.color);
}

void SpriteRenderer::end(rhi::CommandBuffer& commandBuffer)
{
    if (!m_ready || !m_frameOpen)
    {
        return;
    }

    closeBatch();

    if (!m_batches.empty() &&
        m_vertexBuffer->write(m_vertices.data(), m_vertices.size() * sizeof(SpriteVertex), 0))
    {
        commandBuffer.bindPipeline(*m_pipeline);
        commandBuffer.bindVertexBuffer(*m_vertexBuffer);
        commandBuffer.pushConstants(m_projection.elements.data(),
                                    static_cast<std::uint32_t>(sizeof(math::Mat4)));

        for (const Batch& batch : m_batches)
        {
            if (batch.clipped)
            {
                commandBuffer.setScissor(static_cast<std::uint32_t>(batch.clipX),
                                         static_cast<std::uint32_t>(batch.clipY),
                                         static_cast<std::uint32_t>(batch.clipWidth),
                                         static_cast<std::uint32_t>(batch.clipHeight));
            }
            else
            {
                commandBuffer.setScissor(0, 0, 16384, 16384);
            }

            commandBuffer.bindTexture(*batch.texture, 0);
            commandBuffer.draw(batch.vertexCount, batch.firstVertex);
        }
    }

    m_vertices.clear();
    m_batches.clear();
    m_currentTexture = nullptr;
    m_currentBatchStart = 0;
    m_frameOpen = false;
}

void SpriteRenderer::appendVertex(const math::Vec2& position,
                                  float u,
                                  float v,
                                  const math::Vec4& color)
{
    m_vertices.push_back(SpriteVertex{position, math::Vec2{u, v}, color});
}

void SpriteRenderer::closeBatch()
{
    if (m_currentTexture == nullptr)
    {
        return;
    }

    const std::uint32_t vertexCount =
        static_cast<std::uint32_t>(m_vertices.size()) - m_currentBatchStart;

    if (vertexCount > 0)
    {
        m_batches.push_back(Batch{m_currentTexture,
                                  m_currentBatchStart,
                                  vertexCount,
                                  m_clipX,
                                  m_clipY,
                                  m_clipWidth,
                                  m_clipHeight,
                                  m_clipActive});
    }

    m_currentTexture = nullptr;
}

}
