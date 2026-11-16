#pragma once

#include <cstdint>
#include <memory>
#include <string_view>

#include "rhi/device.hpp"
#include "rhi/device_capabilities.hpp"
#include "rhi/native_window.hpp"
#include "rhi/render_context.hpp"

#if defined(SPY_RENDERER_VULKAN)

#include "vulkan/context/vulkan_context.hpp"

namespace spy::vulkan
{

class VulkanDevice final : public rhi::Device
{
public:
    VulkanDevice() = default;
    ~VulkanDevice() override;

    VulkanDevice(const VulkanDevice&) = delete;
    VulkanDevice& operator=(const VulkanDevice&) = delete;

    [[nodiscard]] bool initialize(const rhi::DeviceDesc& desc) override;
    void shutdown() override;
    void waitIdle() override;

    [[nodiscard]] std::unique_ptr<rhi::RenderContext> createRenderContext(
        const rhi::NativeWindow& window,
        std::uint32_t width,
        std::uint32_t height) override;

    [[nodiscard]] rhi::DeviceCapabilities capabilities() const override;
    [[nodiscard]] bool isValid() const override;
    [[nodiscard]] std::string_view backendName() const override;

private:
    VulkanContext m_context;
    rhi::DeviceDesc m_desc{};
    rhi::DeviceCapabilities m_capabilities{};
    bool m_initialized = false;
};

}

#endif
