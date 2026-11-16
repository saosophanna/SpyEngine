#pragma once

#include "rhi/command_buffer.hpp"

#include "vulkan/vulkan_common.hpp"

#if defined(SPY_RENDERER_VULKAN)

namespace spy::vulkan
{

class VulkanContext;

class VulkanCommandBuffer final : public rhi::CommandBuffer
{
public:
    VulkanCommandBuffer() = default;
    ~VulkanCommandBuffer() override;

    VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;
    VulkanCommandBuffer& operator=(const VulkanCommandBuffer&) = delete;

    [[nodiscard]] bool initialize(VulkanContext& context);
    void shutdown();

    [[nodiscard]] bool begin() override;
    [[nodiscard]] bool end() override;
    void beginRenderPass(const rhi::Color& clearColor) override;
    void endRenderPass() override;

    void setRenderTarget(VkRenderPass renderPass, VkFramebuffer framebuffer, VkExtent2D extent);

    [[nodiscard]] VkCommandBuffer handle() const noexcept { return m_commandBuffer; }

private:
    VulkanContext* m_context = nullptr;
    VkCommandPool m_pool = VK_NULL_HANDLE;
    VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkFramebuffer m_framebuffer = VK_NULL_HANDLE;
    VkExtent2D m_extent{};
    bool m_recording = false;
    bool m_renderPassActive = false;
};

}

#endif
