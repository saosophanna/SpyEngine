#if defined(SPY_RENDERER_VULKAN)

#include "vulkan/commands/vulkan_command_buffer.hpp"

#include "vulkan/context/vulkan_context.hpp"

namespace spy::vulkan
{

VulkanCommandBuffer::~VulkanCommandBuffer()
{
    shutdown();
}

bool VulkanCommandBuffer::initialize(VulkanContext& context)
{
    shutdown();

    m_context = &context;

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = context.queueFamilies().graphics;

    if (!checkResult(vkCreateCommandPool(context.device(), &poolInfo, nullptr, &m_pool),
                     "vkCreateCommandPool"))
    {
        m_pool = VK_NULL_HANDLE;
        m_context = nullptr;
        return false;
    }

    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = m_pool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1;

    if (!checkResult(vkAllocateCommandBuffers(context.device(), &allocateInfo, &m_commandBuffer),
                     "vkAllocateCommandBuffers"))
    {
        shutdown();
        return false;
    }

    return true;
}

void VulkanCommandBuffer::shutdown()
{
    if (m_pool != VK_NULL_HANDLE && m_context != nullptr)
    {
        vkDestroyCommandPool(m_context->device(), m_pool, nullptr);
    }

    m_pool = VK_NULL_HANDLE;
    m_commandBuffer = VK_NULL_HANDLE;
    m_renderPass = VK_NULL_HANDLE;
    m_framebuffer = VK_NULL_HANDLE;
    m_extent = VkExtent2D{};
    m_recording = false;
    m_renderPassActive = false;
    m_context = nullptr;
}

bool VulkanCommandBuffer::begin()
{
    if (m_commandBuffer == VK_NULL_HANDLE || m_recording)
    {
        return false;
    }

    if (!checkResult(vkResetCommandPool(m_context->device(), m_pool, 0), "vkResetCommandPool"))
    {
        return false;
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (!checkResult(vkBeginCommandBuffer(m_commandBuffer, &beginInfo),
                     "vkBeginCommandBuffer"))
    {
        return false;
    }

    m_recording = true;
    m_renderPassActive = false;
    return true;
}

bool VulkanCommandBuffer::end()
{
    if (!m_recording)
    {
        return false;
    }

    if (m_renderPassActive)
    {
        endRenderPass();
    }

    if (!checkResult(vkEndCommandBuffer(m_commandBuffer), "vkEndCommandBuffer"))
    {
        m_recording = false;
        return false;
    }

    m_recording = false;
    return true;
}

void VulkanCommandBuffer::beginRenderPass(const rhi::Color& clearColor)
{
    if (!m_recording || m_renderPassActive ||
        m_renderPass == VK_NULL_HANDLE || m_framebuffer == VK_NULL_HANDLE)
    {
        return;
    }

    VkClearValue clearValue{};
    clearValue.color.float32[0] = clearColor.r;
    clearValue.color.float32[1] = clearColor.g;
    clearValue.color.float32[2] = clearColor.b;
    clearValue.color.float32[3] = clearColor.a;

    VkRenderPassBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.renderPass = m_renderPass;
    beginInfo.framebuffer = m_framebuffer;
    beginInfo.renderArea.offset = {0, 0};
    beginInfo.renderArea.extent = m_extent;
    beginInfo.clearValueCount = 1;
    beginInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(m_commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_extent.width);
    viewport.height = static_cast<float>(m_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_extent;
    vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);

    m_renderPassActive = true;
}

void VulkanCommandBuffer::endRenderPass()
{
    if (!m_renderPassActive)
    {
        return;
    }

    vkCmdEndRenderPass(m_commandBuffer);
    m_renderPassActive = false;
}

void VulkanCommandBuffer::setRenderTarget(VkRenderPass renderPass,
                                          VkFramebuffer framebuffer,
                                          VkExtent2D extent)
{
    m_renderPass = renderPass;
    m_framebuffer = framebuffer;
    m_extent = extent;
}

}

#endif
