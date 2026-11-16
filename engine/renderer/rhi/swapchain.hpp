#pragma once

#include <cstdint>

#include "rhi/texture.hpp"

namespace spy::rhi
{

struct SwapchainDesc
{
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint32_t imageCount = 2;
    TextureFormat format = TextureFormat::BGRA8_SRGB;
    bool vsync = true;
};

class Swapchain
{
public:
    virtual ~Swapchain() = default;

    Swapchain(const Swapchain&) = delete;
    Swapchain& operator=(const Swapchain&) = delete;

    [[nodiscard]] virtual std::uint32_t width() const = 0;
    [[nodiscard]] virtual std::uint32_t height() const = 0;
    [[nodiscard]] virtual std::uint32_t imageCount() const = 0;
    [[nodiscard]] virtual std::uint32_t currentImageIndex() const = 0;
    [[nodiscard]] virtual TextureFormat format() const = 0;

protected:
    Swapchain() = default;
};

}
