#pragma once

#include <cstdint>
#include <vector>

#include "rhi/swapchain.hpp"

#include "vulkan/vulkan_common.hpp"

#if defined(SPY_RENDERER_VULKAN)

namespace spy::vulkan
{

class VulkanContext;

class VulkanSwapchain final : public rhi::Swapchain
{
public:
    VulkanSwapchain() = default;
    ~VulkanSwapchain() override;

    VulkanSwapchain(const VulkanSwapchain&) = delete;
    VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;

    [[nodiscard]] bool initialize(VulkanContext& context,
                                  std::uint32_t width,
                                  std::uint32_t height,
                                  bool vsync);
    [[nodiscard]] bool recreate(std::uint32_t width, std::uint32_t height);
    void shutdown();

    [[nodiscard]] bool acquire(VkSemaphore semaphore, std::uint32_t& imageIndex);
    [[nodiscard]] VkResult present(VkQueue queue, VkSemaphore semaphore);

    [[nodiscard]] std::uint32_t width() const override;
    [[nodiscard]] std::uint32_t height() const override;
    [[nodiscard]] std::uint32_t imageCount() const override;
    [[nodiscard]] std::uint32_t currentImageIndex() const override;
    [[nodiscard]] rhi::TextureFormat format() const override;

    [[nodiscard]] VkSwapchainKHR handle() const noexcept { return m_swapchain; }
    [[nodiscard]] VkRenderPass renderPass() const noexcept { return m_renderPass; }
    [[nodiscard]] VkFramebuffer framebuffer(std::uint32_t index) const;
    [[nodiscard]] VkExtent2D extent() const noexcept { return m_extent; }

private:
    [[nodiscard]] bool createSwapchain(std::uint32_t width, std::uint32_t height);
    [[nodiscard]] bool createImageViews();
    [[nodiscard]] bool createRenderPass();
    [[nodiscard]] bool createFramebuffers();
    void destroySwapchainObjects();
    void destroyRenderPass();

    VulkanContext* m_context = nullptr;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkFormat m_format = VK_FORMAT_UNDEFINED;
    VkColorSpaceKHR m_colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    VkExtent2D m_extent{};
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;
    std::vector<VkFramebuffer> m_framebuffers;
    std::uint32_t m_currentImage = 0;
    bool m_vsync = true;
};

}

#endif
