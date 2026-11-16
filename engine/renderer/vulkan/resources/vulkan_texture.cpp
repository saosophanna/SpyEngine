#if defined(SPY_RENDERER_VULKAN)

#include "vulkan/resources/vulkan_texture.hpp"

#include <cstring>

#include "vulkan/context/vulkan_context.hpp"
#include "vulkan/resources/vulkan_upload.hpp"

namespace spy::vulkan
{

namespace
{

VkFormat toVulkanFormat(rhi::TextureFormat format)
{
    switch (format)
    {
        case rhi::TextureFormat::BGRA8_UNorm:
            return VK_FORMAT_B8G8R8A8_UNORM;
        case rhi::TextureFormat::RGBA8_SRGB:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case rhi::TextureFormat::BGRA8_SRGB:
            return VK_FORMAT_B8G8R8A8_SRGB;
        case rhi::TextureFormat::R8_UNorm:
            return VK_FORMAT_R8_UNORM;
        case rhi::TextureFormat::Depth32F:
            return VK_FORMAT_D32_SFLOAT;
        case rhi::TextureFormat::RGBA8_UNorm:
        default:
            return VK_FORMAT_R8G8B8A8_UNORM;
    }
}

std::size_t bytesPerPixel(rhi::TextureFormat format)
{
    switch (format)
    {
        case rhi::TextureFormat::R8_UNorm:
            return 1;
        case rhi::TextureFormat::Depth32F:
            return 4;
        default:
            return 4;
    }
}

VkFilter toVulkanFilter(rhi::TextureFilter filter)
{
    return filter == rhi::TextureFilter::Nearest ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
}

VkSamplerAddressMode toVulkanWrap(rhi::TextureWrap wrap)
{
    switch (wrap)
    {
        case rhi::TextureWrap::Repeat:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case rhi::TextureWrap::MirroredRepeat:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case rhi::TextureWrap::ClampToEdge:
        default:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    }
}

}

VulkanTexture::~VulkanTexture()
{
    shutdown();
}

bool VulkanTexture::initialize(VulkanContext& context,
                               const rhi::TextureDesc& desc,
                               VkDescriptorSetLayout setLayout,
                               VkDescriptorPool descriptorPool)
{
    shutdown();

    if (desc.width == 0 || desc.height == 0)
    {
        return false;
    }

    m_context = &context;
    m_width = desc.width;
    m_height = desc.height;
    m_mipLevels = desc.mipLevels == 0 ? 1 : desc.mipLevels;
    m_format = desc.format;

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent = {m_width, m_height, 1};
    imageInfo.mipLevels = m_mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = toVulkanFormat(m_format);
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (desc.isRenderTarget)
    {
        imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }

    if (!checkResult(vkCreateImage(context.device(), &imageInfo, nullptr, &m_image),
                     "vkCreateImage"))
    {
        shutdown();
        return false;
    }

    VkMemoryRequirements requirements{};
    vkGetImageMemoryRequirements(context.device(), m_image, &requirements);

    const std::uint32_t memoryType =
        findMemoryType(context, requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (memoryType == 0xFFFFFFFFu)
    {
        shutdown();
        return false;
    }

    VkMemoryAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = requirements.size;
    allocateInfo.memoryTypeIndex = memoryType;

    if (!checkResult(vkAllocateMemory(context.device(), &allocateInfo, nullptr, &m_memory),
                     "vkAllocateMemory") ||
        !checkResult(vkBindImageMemory(context.device(), m_image, m_memory, 0),
                     "vkBindImageMemory"))
    {
        shutdown();
        return false;
    }

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = toVulkanFormat(m_format);
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = m_mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (!checkResult(vkCreateImageView(context.device(), &viewInfo, nullptr, &m_view),
                     "vkCreateImageView"))
    {
        shutdown();
        return false;
    }

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = toVulkanFilter(desc.filter);
    samplerInfo.minFilter = toVulkanFilter(desc.filter);
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = toVulkanWrap(desc.wrap);
    samplerInfo.addressModeV = toVulkanWrap(desc.wrap);
    samplerInfo.addressModeW = toVulkanWrap(desc.wrap);
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(m_mipLevels);
    samplerInfo.maxAnisotropy = 1.0f;

    if (!checkResult(vkCreateSampler(context.device(), &samplerInfo, nullptr, &m_sampler),
                     "vkCreateSampler"))
    {
        shutdown();
        return false;
    }

    if (!createDescriptorSet(setLayout, descriptorPool))
    {
        shutdown();
        return false;
    }

    if (desc.initialPixels != nullptr)
    {
        if (!uploadInitialPixels(desc.initialPixels, expectedUploadBytes()))
        {
            shutdown();
            return false;
        }
    }

    return true;
}

void VulkanTexture::shutdown()
{
    if (m_context == nullptr)
    {
        return;
    }

    if (m_sampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(m_context->device(), m_sampler, nullptr);
        m_sampler = VK_NULL_HANDLE;
    }

    if (m_view != VK_NULL_HANDLE)
    {
        vkDestroyImageView(m_context->device(), m_view, nullptr);
        m_view = VK_NULL_HANDLE;
    }

    if (m_image != VK_NULL_HANDLE)
    {
        vkDestroyImage(m_context->device(), m_image, nullptr);
        m_image = VK_NULL_HANDLE;
    }

    if (m_memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(m_context->device(), m_memory, nullptr);
        m_memory = VK_NULL_HANDLE;
    }

    m_descriptorSet = VK_NULL_HANDLE;
    m_descriptorPool = VK_NULL_HANDLE;
    m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    m_context = nullptr;
    m_width = 0;
    m_height = 0;
    m_mipLevels = 1;
}

std::uint32_t VulkanTexture::width() const
{
    return m_width;
}

std::uint32_t VulkanTexture::height() const
{
    return m_height;
}

rhi::TextureFormat VulkanTexture::format() const
{
    return m_format;
}

bool VulkanTexture::upload(const void* pixels, std::size_t bytes)
{
    if (m_context == nullptr || pixels == nullptr || bytes < expectedUploadBytes())
    {
        return false;
    }

    VkBuffer staging = VK_NULL_HANDLE;
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;

    if (!createVulkanBuffer(*m_context,
                            static_cast<VkDeviceSize>(bytes),
                            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                            staging,
                            stagingMemory))
    {
        return false;
    }

    void* stagingData = nullptr;

    if (!checkResult(vkMapMemory(m_context->device(),
                                 stagingMemory,
                                 0,
                                 VK_WHOLE_SIZE,
                                 0,
                                 &stagingData),
                     "vkMapMemory"))
    {
        destroyVulkanBuffer(*m_context, staging, stagingMemory);
        return false;
    }

    std::memcpy(stagingData, pixels, bytes);
    vkUnmapMemory(m_context->device(), stagingMemory);

    const VkImageLayout previousLayout = m_layout;

    const bool uploaded = submitImmediate(*m_context, [&](VkCommandBuffer commandBuffer)
    {
        if (!transitionImageLayout(commandBuffer, m_image, previousLayout,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL))
        {
            return false;
        }

        if (!copyBufferToImage(commandBuffer, staging, m_image, m_width, m_height))
        {
            return false;
        }

        return transitionImageLayout(commandBuffer,
                                     m_image,
                                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    });

    destroyVulkanBuffer(*m_context, staging, stagingMemory);

    if (uploaded)
    {
        m_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    return uploaded;
}

VkDescriptorSet VulkanTexture::descriptorSet() const noexcept
{
    return m_descriptorSet;
}

bool VulkanTexture::uploadInitialPixels(const void* pixels, std::size_t bytes)
{
    return upload(pixels, bytes);
}

bool VulkanTexture::createDescriptorSet(VkDescriptorSetLayout setLayout,
                                        VkDescriptorPool descriptorPool)
{
    if (setLayout == VK_NULL_HANDLE || descriptorPool == VK_NULL_HANDLE)
    {
        return false;
    }

    VkDescriptorSetAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = descriptorPool;
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &setLayout;

    if (!checkResult(vkAllocateDescriptorSets(m_context->device(), &allocateInfo, &m_descriptorSet),
                     "vkAllocateDescriptorSets"))
    {
        m_descriptorSet = VK_NULL_HANDLE;
        return false;
    }

    m_descriptorPool = descriptorPool;

    VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler = m_sampler;
    imageInfo.imageView = m_view;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = m_descriptorSet;
    write.dstBinding = 0;
    write.dstArrayElement = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(m_context->device(), 1, &write, 0, nullptr);
    return true;
}

std::size_t VulkanTexture::expectedUploadBytes() const
{
    return static_cast<std::size_t>(m_width) * static_cast<std::size_t>(m_height) *
           bytesPerPixel(m_format);
}

}

#endif
