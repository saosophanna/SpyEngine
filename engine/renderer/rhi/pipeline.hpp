#pragma once

#include <cstddef>
#include <cstdint>

namespace spy::rhi
{

enum class VertexFormat
{
    Float2,
    Float3,
    Float4,
};

enum class PrimitiveTopology
{
    TriangleList,
    TriangleStrip,
    LineList,
};

enum class BlendMode
{
    None,
    Alpha,
    Additive,
};

enum class CullMode
{
    None,
    Back,
    Front,
};

enum class ShaderStage
{
    Vertex,
    Fragment,
};

enum class DepthMode
{
    Disabled,
    ReadOnly,
    ReadWrite,
};

struct VertexAttribute
{
    std::uint32_t location = 0;
    VertexFormat format = VertexFormat::Float3;
    std::uint32_t offset = 0;
};

struct ShaderModule
{
    ShaderStage stage = ShaderStage::Vertex;
    const void* code = nullptr;
    std::size_t size = 0;
};

struct PipelineDesc
{
    const ShaderModule* stages = nullptr;
    std::uint32_t stageCount = 0;
    const VertexAttribute* attributes = nullptr;
    std::uint32_t attributeCount = 0;
    std::uint32_t vertexStride = 0;
    PrimitiveTopology topology = PrimitiveTopology::TriangleList;
    BlendMode blend = BlendMode::Alpha;
    CullMode cull = CullMode::None;
    DepthMode depth = DepthMode::Disabled;
    std::uint32_t pushConstantSize = 0;
    bool hasTexture = true;
};

class Pipeline
{
public:
    virtual ~Pipeline() = default;

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    [[nodiscard]] virtual bool isValid() const = 0;

protected:
    Pipeline() = default;
};

}
