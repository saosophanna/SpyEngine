#pragma once

#include <cstddef>
#include <cstdint>

namespace spy::rhi {

class Buffer;

enum class VertexFormat {
    Float2,
    Float3,
    Float4,
};

enum class PrimitiveTopology {
    TriangleList,
    TriangleStrip,
    LineList,
};

enum class BlendMode {
    None,
    Alpha,
    Additive,
};

enum class CullMode {
    None,
    Back,
    Front,
};

struct VertexAttribute {
    std::uint32_t location = 0;
    VertexFormat  format   = VertexFormat::Float3;
    std::uint32_t offset   = 0;
};

struct PipelineDesc {
    const void*      shaderStage    = nullptr;
    std::size_t      shaderSize     = 0;
    VertexAttribute* attributes     = nullptr;
    std::uint32_t    attributeCount = 0;
    std::uint32_t    vertexStride   = 0;
    PrimitiveTopology topology      = PrimitiveTopology::TriangleList;
    BlendMode         blend         = BlendMode::Alpha;
    CullMode          cull          = CullMode::Back;
    bool              depthTest     = true;
    bool              depthWrite    = true;
};

class Pipeline {
public:
    virtual ~Pipeline() = default;

    Pipeline(const Pipeline&)            = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    virtual bool isValid() const = 0;

protected:
    Pipeline() = default;
};

}
