#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "rhi/device.hpp"
#include "rhi/render_context.hpp"

#include "vulkan/commands/vulkan_command_buffer.hpp"
#include "vulkan/swapchain/vulkan_swapchain.hpp"
#include "vulkan/synchronization/vulkan_sync.hpp"

#if defined(SPY_RENDERER_VULKAN)

namespace spy::vulkan
{

class VulkanContext;

class VulkanRenderContext final : public rhi::RenderContext
{
public:
    VulkanRenderContext(VulkanContext& context,
                        const rhi::DeviceDesc& desc,
                        VkDescriptorSetLayout textureSetLayout);
    ~VulkanRenderContext() override;

    VulkanRenderContext(const VulkanRenderContext&) = delete;
    VulkanRenderContext& operator=(const VulkanRenderContext&) = delete;

    [[nodiscard]] bool initialize(const rhi::NativeWindow& window,
                                  std::uint32_t width,
                                  std::uint32_t height,
                                  bool vsync) override;
    void shutdown() override;

    [[nodiscard]] bool resize(std::uint32_t width, std::uint32_t height) override;
    [[nodiscard]] bool beginFrame() override;
    [[nodiscard]] bool endFrame() override;

    void waitIdle() override;

    [[nodiscard]] std::unique_ptr<rhi::Pipeline> createPipeline(
        const rhi::PipelineDesc& desc) override;

    [[nodiscard]] std::uint32_t width() const override;
    [[nodiscard]] std::uint32_t height() const override;
    [[nodiscard]] std::uint32_t imageCount() const override;
    [[nodiscard]] std::uint32_t currentFrameIndex() const override;

    [[nodiscard]] rhi::CommandBuffer& currentCommandBuffer() override;
    [[nodiscard]] rhi::Swapchain& swapchain() override;

private:
    struct Frame
    {
        std::unique_ptr<VulkanCommandBuffer> commandBuffer;
        std::unique_ptr<VulkanFence> inFlightFence;
        std::unique_ptr<VulkanSemaphore> imageAvailable;
    };

    [[nodiscard]] bool createFrameResources();
    void destroyFrameResources();
    [[nodiscard]] bool createPerImageSemaphores();
    void destroyPerImageSemaphores();
    [[nodiscard]] bool recreateSwapchain();

    VulkanContext* m_context = nullptr;
    VulkanSwapchain m_swapchain;
    VkDescriptorSetLayout m_textureSetLayout = VK_NULL_HANDLE;
    std::vector<Frame> m_frames;
    std::vector<std::unique_ptr<VulkanSemaphore>> m_renderFinished;
    std::uint32_t m_currentFrame = 0;
    std::uint32_t m_width = 0;
    std::uint32_t m_height = 0;
    std::uint32_t m_framesInFlight = 2;
    bool m_vsync = true;
    bool m_initialized = false;
};

}

#endif
