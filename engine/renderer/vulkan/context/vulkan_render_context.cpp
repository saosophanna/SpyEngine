#if defined(SPY_RENDERER_VULKAN)

#include "vulkan/context/vulkan_render_context.hpp"

#include <algorithm>
#include <cstdint>

#include "core/logging/log_macros.hpp"

#include "vulkan/context/vulkan_context.hpp"
#include "vulkan/resources/vulkan_pipeline.hpp"

namespace spy::vulkan
{

VulkanRenderContext::VulkanRenderContext(VulkanContext& context,
                                         const rhi::DeviceDesc& desc,
                                         VkDescriptorSetLayout textureSetLayout)
    : m_context(&context)
    , m_textureSetLayout(textureSetLayout)
    , m_framesInFlight(std::clamp<std::uint32_t>(desc.framesInFlight, 1, 3))
    , m_vsync(desc.enableVsync)
{
}

VulkanRenderContext::~VulkanRenderContext()
{
    shutdown();
}

bool VulkanRenderContext::initialize(const rhi::NativeWindow& window,
                                     std::uint32_t width,
                                     std::uint32_t height,
                                     bool vsync)
{
    (void)window;

    if (m_initialized)
    {
        return true;
    }

    if (m_context == nullptr || !m_context->valid() || m_context->surface() == VK_NULL_HANDLE)
    {
        SPY_LOG_ERROR("Vulkan render context requires an initialized device and surface");
        return false;
    }

    m_width = width;
    m_height = height;
    m_vsync = vsync;

    if (!m_swapchain.initialize(*m_context, width, height, vsync))
    {
        return false;
    }

    if (!createFrameResources())
    {
        m_swapchain.shutdown();
        return false;
    }

    if (!createPerImageSemaphores())
    {
        destroyFrameResources();
        m_swapchain.shutdown();
        return false;
    }

    m_initialized = true;
    return true;
}

void VulkanRenderContext::shutdown()
{
    if (m_context == nullptr)
    {
        return;
    }

    if (m_context->valid())
    {
        m_context->waitIdle();
    }

    destroyPerImageSemaphores();
    destroyFrameResources();
    m_swapchain.shutdown();

    m_initialized = false;
    m_currentFrame = 0;
}

bool VulkanRenderContext::resize(std::uint32_t width, std::uint32_t height)
{
    if (!m_initialized)
    {
        m_width = width;
        m_height = height;
        return true;
    }

    if (width == 0 || height == 0)
    {
        m_width = 0;
        m_height = 0;
        return true;
    }

    if (width == m_swapchain.width() && height == m_swapchain.height())
    {
        m_width = width;
        m_height = height;
        return true;
    }

    m_width = width;
    m_height = height;
    return recreateSwapchain();
}

bool VulkanRenderContext::beginFrame()
{
    if (!m_initialized || m_width == 0 || m_height == 0 || m_frames.empty())
    {
        return false;
    }

    Frame& frame = m_frames[m_currentFrame];

    if (!frame.inFlightFence->wait())
    {
        SPY_LOG_ERROR("Vulkan frame fence wait failed");
        return false;
    }

    std::uint32_t imageIndex = 0;

    if (!m_swapchain.acquire(frame.imageAvailable->handle(), imageIndex))
    {
        if (!recreateSwapchain())
        {
            return false;
        }

        if (!m_swapchain.acquire(frame.imageAvailable->handle(), imageIndex))
        {
            return false;
        }
    }

    if (!frame.commandBuffer->begin())
    {
        m_initialized = false;
        return false;
    }

    frame.commandBuffer->setRenderTarget(m_swapchain.renderPass(),
                                         m_swapchain.framebuffer(imageIndex),
                                         m_swapchain.extent());

    return true;
}

bool VulkanRenderContext::endFrame()
{
    if (!m_initialized || m_frames.empty())
    {
        return false;
    }

    Frame& frame = m_frames[m_currentFrame];

    if (!frame.commandBuffer->end())
    {
        return false;
    }

    const std::uint32_t imageIndex = m_swapchain.currentImageIndex();

    if (imageIndex >= m_renderFinished.size())
    {
        return false;
    }

    const VkSemaphore imageAvailable = frame.imageAvailable->handle();
    const VkSemaphore renderFinished = m_renderFinished[imageIndex]->handle();
    const VkCommandBuffer commandBuffer = frame.commandBuffer->handle();
    const VkFence fence = frame.inFlightFence->handle();

    frame.inFlightFence->reset();

    const VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imageAvailable;
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinished;

    if (!checkResult(vkQueueSubmit(m_context->graphicsQueue(), 1, &submitInfo, fence),
                     "vkQueueSubmit"))
    {
        m_initialized = false;
        return false;
    }

    const VkResult presentResult = m_swapchain.present(m_context->presentQueue(), renderFinished);

    m_currentFrame = (m_currentFrame + 1) % m_framesInFlight;

    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR)
    {
        return true;
    }

    return checkResult(presentResult, "vkQueuePresentKHR");
}

void VulkanRenderContext::waitIdle()
{
    if (m_context != nullptr)
    {
        m_context->waitIdle();
    }
}

std::unique_ptr<rhi::Pipeline> VulkanRenderContext::createPipeline(
    const rhi::PipelineDesc& desc)
{
    if (!m_initialized || m_context == nullptr)
    {
        return nullptr;
    }

    auto pipeline = std::make_unique<VulkanPipeline>();

    if (!pipeline->initialize(*m_context,
                              desc,
                              m_swapchain.renderPass(),
                              m_textureSetLayout))
    {
        return nullptr;
    }

    return pipeline;
}

std::uint32_t VulkanRenderContext::width() const
{
    return m_width;
}

std::uint32_t VulkanRenderContext::height() const
{
    return m_height;
}

std::uint32_t VulkanRenderContext::imageCount() const
{
    return m_swapchain.imageCount();
}

std::uint32_t VulkanRenderContext::currentFrameIndex() const
{
    return m_currentFrame;
}

rhi::CommandBuffer& VulkanRenderContext::currentCommandBuffer()
{
    return *m_frames[m_currentFrame].commandBuffer;
}

rhi::Swapchain& VulkanRenderContext::swapchain()
{
    return m_swapchain;
}

bool VulkanRenderContext::createFrameResources()
{
    m_frames.clear();
    m_frames.resize(m_framesInFlight);
    m_currentFrame = 0;

    for (Frame& frame : m_frames)
    {
        frame.commandBuffer = std::make_unique<VulkanCommandBuffer>();
        frame.inFlightFence = std::make_unique<VulkanFence>();
        frame.imageAvailable = std::make_unique<VulkanSemaphore>();

        if (!frame.commandBuffer->initialize(*m_context) ||
            !frame.inFlightFence->initialize(m_context->device(), true) ||
            !frame.imageAvailable->initialize(m_context->device()))
        {
            destroyFrameResources();
            return false;
        }
    }

    return true;
}

void VulkanRenderContext::destroyFrameResources()
{
    m_frames.clear();
    m_currentFrame = 0;
}

bool VulkanRenderContext::createPerImageSemaphores()
{
    destroyPerImageSemaphores();

    const std::uint32_t count = m_swapchain.imageCount();
    m_renderFinished.resize(count);

    for (std::size_t index = 0; index < count; ++index)
    {
        m_renderFinished[index] = std::make_unique<VulkanSemaphore>();

        if (!m_renderFinished[index]->initialize(m_context->device()))
        {
            destroyPerImageSemaphores();
            return false;
        }
    }

    return true;
}

void VulkanRenderContext::destroyPerImageSemaphores()
{
    m_renderFinished.clear();
}

bool VulkanRenderContext::recreateSwapchain()
{
    if (m_context == nullptr || m_width == 0 || m_height == 0)
    {
        return false;
    }

    m_context->waitIdle();
    destroyPerImageSemaphores();

    if (!m_swapchain.recreate(m_width, m_height))
    {
        return false;
    }

    return createPerImageSemaphores();
}

}

#endif
