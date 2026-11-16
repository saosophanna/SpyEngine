#pragma once

#include <cstddef>
#include <cstdint>

namespace spy::rhi
{

enum class TextureFormat
{
    RGBA8_UNorm,
    BGRA8_UNorm,
    RGBA8_SRGB,
    BGRA8_SRGB,
    R8_UNorm,
    Depth32F,
};

enum class TextureFilter
{
    Nearest,
    Linear,
};

enum class TextureWrap
{
    Repeat,
    ClampToEdge,
    MirroredRepeat,
};

struct TextureDesc
{
    std::uint32_t width = 1;
    std::uint32_t height = 1;
    std::uint32_t mipLevels = 1;
    TextureFormat format = TextureFormat::RGBA8_UNorm;
    TextureFilter filter = TextureFilter::Linear;
    TextureWrap wrap = TextureWrap::ClampToEdge;
    bool isRenderTarget = false;
    const void* initialPixels = nullptr;
};

class Texture
{
public:
    virtual ~Texture() = default;

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    [[nodiscard]] virtual std::uint32_t width() const = 0;
    [[nodiscard]] virtual std::uint32_t height() const = 0;
    [[nodiscard]] virtual TextureFormat format() const = 0;

    virtual bool upload(const void* pixels, std::size_t bytes) = 0;

protected:
    Texture() = default;
};

}
