#pragma once

#include <cstddef>
#include <cstdint>

#include "rhi/texture.hpp"

#include "vulkan/vulkan_common.hpp"

#if defined(SPY_RENDERER_VULKAN)

namespace spy::vulkan
{

class VulkanContext;

class VulkanTexture final : public rhi::Texture
{
public:
    VulkanTexture() = default;
    ~VulkanTexture() override;

    VulkanTexture(const VulkanTexture&) = delete;
    VulkanTexture& operator=(const VulkanTexture&) = delete;

    [[nodiscard]] bool initialize(VulkanContext& context,
                                  const rhi::TextureDesc& desc,
                                  VkDescriptorSetLayout setLayout,
                                  VkDescriptorPool descriptorPool);
    void shutdown();

    [[nodiscard]] std::uint32_t width() const override;
    [[nodiscard]] std::uint32_t height() const override;
    [[nodiscard]] rhi::TextureFormat format() const override;

    [[nodiscard]] bool upload(const void* pixels, std::size_t bytes) override;

    [[nodiscard]] VkDescriptorSet descriptorSet() const noexcept;

private:
    [[nodiscard]] bool uploadInitialPixels(const void* pixels, std::size_t bytes);
    [[nodiscard]] bool createDescriptorSet(VkDescriptorSetLayout setLayout,
                                           VkDescriptorPool descriptorPool);
    [[nodiscard]] std::size_t expectedUploadBytes() const;

    VulkanContext* m_context = nullptr;
    VkImage m_image = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;
    VkImageView m_view = VK_NULL_HANDLE;
    VkSampler m_sampler = VK_NULL_HANDLE;
    VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    std::uint32_t m_width = 0;
    std::uint32_t m_height = 0;
    std::uint32_t m_mipLevels = 1;
    rhi::TextureFormat m_format = rhi::TextureFormat::RGBA8_UNorm;
};

}

#endif
