#if defined(SPY_RENDERER_VULKAN)

#include "vulkan/swapchain/vulkan_swapchain.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <vector>

#include "core/logging/log_macros.hpp"

#include "vulkan/context/vulkan_context.hpp"

namespace spy::vulkan
{

namespace
{

VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
    for (const VkSurfaceFormatKHR& format : formats)
    {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return format;
        }
    }

    for (const VkSurfaceFormatKHR& format : formats)
    {
        if (format.format == VK_FORMAT_R8G8B8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return format;
        }
    }

    return formats.front();
}

VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& modes, bool vsync)
{
    if (!vsync)
    {
        for (VkPresentModeKHR mode : modes)
        {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return mode;
            }
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkCompositeAlphaFlagBitsKHR chooseCompositeAlpha(VkCompositeAlphaFlagsKHR supported)
{
    const VkCompositeAlphaFlagBitsKHR candidates[] = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };

    for (VkCompositeAlphaFlagBitsKHR candidate : candidates)
    {
        if ((supported & candidate) != 0)
        {
            return candidate;
        }
    }

    return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
}

}

VulkanSwapchain::~VulkanSwapchain()
{
    shutdown();
}

bool VulkanSwapchain::initialize(VulkanContext& context,
                                 std::uint32_t width,
                                 std::uint32_t height,
                                 bool vsync)
{
    m_context = &context;
    m_vsync = vsync;

    if (!createSwapchain(width, height))
    {
        return false;
    }

    if (!createImageViews() || !createRenderPass() || !createFramebuffers())
    {
        shutdown();
        return false;
    }

    return true;
}

bool VulkanSwapchain::recreate(std::uint32_t width, std::uint32_t height)
{
    destroySwapchainObjects();

    if (!createSwapchain(width, height))
    {
        return false;
    }

    if (!createImageViews() || !createRenderPass() || !createFramebuffers())
    {
        destroySwapchainObjects();
        return false;
    }

    return true;
}

void VulkanSwapchain::shutdown()
{
    destroySwapchainObjects();
    destroyRenderPass();
    m_images.clear();
    m_context = nullptr;
}

bool VulkanSwapchain::acquire(VkSemaphore semaphore, std::uint32_t& imageIndex)
{
    if (m_swapchain == VK_NULL_HANDLE)
    {
        return false;
    }

    const VkResult result = vkAcquireNextImageKHR(m_context->device(),
                                                  m_swapchain,
                                                  std::numeric_limits<std::uint64_t>::max(),
                                                  semaphore,
                                                  VK_NULL_HANDLE,
                                                  &imageIndex);

    if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR)
    {
        m_currentImage = imageIndex;
        return true;
    }

    if (result != VK_ERROR_OUT_OF_DATE_KHR)
    {
        (void)checkResult(result, "vkAcquireNextImageKHR");
    }

    return false;
}

VkResult VulkanSwapchain::present(VkQueue queue, VkSemaphore semaphore)
{
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &semaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapchain;
    presentInfo.pImageIndices = &m_currentImage;

    return vkQueuePresentKHR(queue, &presentInfo);
}

std::uint32_t VulkanSwapchain::width() const
{
    return m_extent.width;
}

std::uint32_t VulkanSwapchain::height() const
{
    return m_extent.height;
}

std::uint32_t VulkanSwapchain::imageCount() const
{
    return static_cast<std::uint32_t>(m_images.size());
}

std::uint32_t VulkanSwapchain::currentImageIndex() const
{
    return m_currentImage;
}

rhi::TextureFormat VulkanSwapchain::format() const
{
    switch (m_format)
    {
    case VK_FORMAT_B8G8R8A8_SRGB:
        return rhi::TextureFormat::BGRA8_SRGB;
    case VK_FORMAT_R8G8B8A8_SRGB:
        return rhi::TextureFormat::RGBA8_SRGB;
    case VK_FORMAT_B8G8R8A8_UNORM:
        return rhi::TextureFormat::BGRA8_UNorm;
    default:
        return rhi::TextureFormat::RGBA8_UNorm;
    }
}

VkFramebuffer VulkanSwapchain::framebuffer(std::uint32_t index) const
{
    if (index >= m_framebuffers.size())
    {
        return VK_NULL_HANDLE;
    }

    return m_framebuffers[index];
}

bool VulkanSwapchain::createSwapchain(std::uint32_t width, std::uint32_t height)
{
    if (m_context == nullptr || width == 0 || height == 0)
    {
        return false;
    }

    VkSurfaceCapabilitiesKHR capabilities{};
    if (!checkResult(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                         m_context->physicalDevice(), m_context->surface(), &capabilities),
                     "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"))
    {
        return false;
    }

    std::uint32_t formatCount = 0;
    if (!checkResult(vkGetPhysicalDeviceSurfaceFormatsKHR(
                         m_context->physicalDevice(), m_context->surface(), &formatCount, nullptr),
                     "vkGetPhysicalDeviceSurfaceFormatsKHR"))
    {
        return false;
    }

    if (formatCount == 0)
    {
        SPY_LOG_ERROR("Vulkan surface reports no supported formats");
        return false;
    }

    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    if (!checkResult(vkGetPhysicalDeviceSurfaceFormatsKHR(m_context->physicalDevice(),
                                                          m_context->surface(),
                                                          &formatCount,
                                                          formats.data()),
                     "vkGetPhysicalDeviceSurfaceFormatsKHR"))
    {
        return false;
    }

    std::uint32_t presentModeCount = 0;
    if (!checkResult(vkGetPhysicalDeviceSurfacePresentModesKHR(
                         m_context->physicalDevice(),
                         m_context->surface(),
                         &presentModeCount,
                         nullptr),
                     "vkGetPhysicalDeviceSurfacePresentModesKHR"))
    {
        return false;
    }

    if (presentModeCount == 0)
    {
        SPY_LOG_ERROR("Vulkan surface reports no present modes");
        return false;
    }

    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    if (!checkResult(vkGetPhysicalDeviceSurfacePresentModesKHR(m_context->physicalDevice(),
                                                               m_context->surface(),
                                                               &presentModeCount,
                                                               presentModes.data()),
                     "vkGetPhysicalDeviceSurfacePresentModesKHR"))
    {
        return false;
    }

    const VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(formats);
    m_format = surfaceFormat.format;
    m_colorSpace = surfaceFormat.colorSpace;

    if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
    {
        m_extent = capabilities.currentExtent;
    }
    else
    {
        m_extent.width = std::clamp(width,
                                    capabilities.minImageExtent.width,
                                    capabilities.maxImageExtent.width);
        m_extent.height = std::clamp(height,
                                     capabilities.minImageExtent.height,
                                     capabilities.maxImageExtent.height);
    }

    std::uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
    {
        imageCount = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_context->surface();
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = m_format;
    createInfo.imageColorSpace = m_colorSpace;
    createInfo.imageExtent = m_extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = chooseCompositeAlpha(capabilities.supportedCompositeAlpha);
    createInfo.presentMode = choosePresentMode(presentModes, m_vsync);
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (!checkResult(vkCreateSwapchainKHR(m_context->device(), &createInfo, nullptr, &m_swapchain),
                     "vkCreateSwapchainKHR"))
    {
        m_swapchain = VK_NULL_HANDLE;
        return false;
    }

    std::uint32_t swapchainImageCount = 0;
    if (!checkResult(vkGetSwapchainImagesKHR(
                         m_context->device(), m_swapchain, &swapchainImageCount, nullptr),
                     "vkGetSwapchainImagesKHR"))
    {
        return false;
    }

    m_images.resize(swapchainImageCount);
    if (!checkResult(vkGetSwapchainImagesKHR(
                         m_context->device(), m_swapchain, &swapchainImageCount, m_images.data()),
                     "vkGetSwapchainImagesKHR"))
    {
        return false;
    }

    m_currentImage = 0;
    return true;
}

bool VulkanSwapchain::createImageViews()
{
    m_imageViews.resize(m_images.size(), VK_NULL_HANDLE);

    for (std::size_t index = 0; index < m_images.size(); ++index)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_images[index];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (!checkResult(vkCreateImageView(
                             m_context->device(), &createInfo, nullptr, &m_imageViews[index]),
                         "vkCreateImageView"))
        {
            return false;
        }
    }

    return true;
}

bool VulkanSwapchain::createRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorReference{};
    colorReference.attachment = 0;
    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorReference;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = 1;
    createInfo.pAttachments = &colorAttachment;
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpass;
    createInfo.dependencyCount = 1;
    createInfo.pDependencies = &dependency;

    return checkResult(
        vkCreateRenderPass(m_context->device(), &createInfo, nullptr, &m_renderPass),
        "vkCreateRenderPass");
}

bool VulkanSwapchain::createFramebuffers()
{
    m_framebuffers.resize(m_imageViews.size(), VK_NULL_HANDLE);

    for (std::size_t index = 0; index < m_imageViews.size(); ++index)
    {
        VkFramebufferCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.renderPass = m_renderPass;
        createInfo.attachmentCount = 1;
        createInfo.pAttachments = &m_imageViews[index];
        createInfo.width = m_extent.width;
        createInfo.height = m_extent.height;
        createInfo.layers = 1;

        if (!checkResult(vkCreateFramebuffer(
                             m_context->device(), &createInfo, nullptr, &m_framebuffers[index]),
                         "vkCreateFramebuffer"))
        {
            return false;
        }
    }

    return true;
}

void VulkanSwapchain::destroySwapchainObjects()
{
    for (VkFramebuffer framebufferHandle : m_framebuffers)
    {
        if (framebufferHandle != VK_NULL_HANDLE)
        {
            vkDestroyFramebuffer(m_context->device(), framebufferHandle, nullptr);
        }
    }
    m_framebuffers.clear();

    for (VkImageView imageView : m_imageViews)
    {
        if (imageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(m_context->device(), imageView, nullptr);
        }
    }
    m_imageViews.clear();

    if (m_swapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(m_context->device(), m_swapchain, nullptr);
        m_swapchain = VK_NULL_HANDLE;
    }
}

void VulkanSwapchain::destroyRenderPass()
{
    if (m_renderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(m_context->device(), m_renderPass, nullptr);
        m_renderPass = VK_NULL_HANDLE;
    }
}

}

#endif
