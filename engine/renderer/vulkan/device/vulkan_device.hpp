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

    [[nodiscard]] std::unique_ptr<rhi::Buffer> createBuffer(
        const rhi::BufferDesc& desc) override;
    [[nodiscard]] std::unique_ptr<rhi::Texture> createTexture(
        const rhi::TextureDesc& desc) override;

    [[nodiscard]] rhi::DeviceCapabilities capabilities() const override;
    [[nodiscard]] bool isValid() const override;
    [[nodiscard]] std::string_view backendName() const override;

private:
    [[nodiscard]] bool createDescriptorResources();
    void destroyDescriptorResources();

    VulkanContext m_context;
    rhi::DeviceDesc m_desc{};
    rhi::DeviceCapabilities m_capabilities{};
    VkDescriptorSetLayout m_textureSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    bool m_initialized = false;
};

}

#endif
