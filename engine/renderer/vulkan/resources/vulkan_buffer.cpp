#if defined(SPY_RENDERER_VULKAN)

#include "vulkan/resources/vulkan_buffer.hpp"

#include <cstring>

#include "vulkan/context/vulkan_context.hpp"
#include "vulkan/resources/vulkan_upload.hpp"

namespace spy::vulkan
{

namespace
{

bool hasUsage(rhi::BufferUsage value, rhi::BufferUsage flag)
{
    const auto mask = static_cast<std::uint32_t>(value) & static_cast<std::uint32_t>(flag);
    return mask != 0u;
}

VkBufferUsageFlags usageFlags(rhi::BufferUsage usage)
{
    VkBufferUsageFlags flags = 0;

    if (hasUsage(usage, rhi::BufferUsage::Vertex))
    {
        flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }

    if (hasUsage(usage, rhi::BufferUsage::Index))
    {
        flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }

    if (hasUsage(usage, rhi::BufferUsage::Uniform))
    {
        flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }

    if (hasUsage(usage, rhi::BufferUsage::Storage))
    {
        flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }

    if (flags == 0)
    {
        flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }

    return flags | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
}

VkMemoryPropertyFlags memoryFlags(rhi::MemoryUsage usage, bool& hostVisible)
{
    switch (usage)
    {
        case rhi::MemoryUsage::CpuToGpu:
            hostVisible = true;
            return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        case rhi::MemoryUsage::GpuToCpu:
            hostVisible = true;
            return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                   VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        case rhi::MemoryUsage::GpuOnly:
        default:
            hostVisible = false;
            return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }
}

}

VulkanBuffer::~VulkanBuffer()
{
    shutdown();
}

bool VulkanBuffer::initialize(VulkanContext& context, const rhi::BufferDesc& desc)
{
    shutdown();

    if (desc.size == 0)
    {
        return false;
    }

    m_context = &context;
    m_size = desc.size;

    const VkMemoryPropertyFlags properties = memoryFlags(desc.memory, m_hostVisible);

    if (!createVulkanBuffer(context,
                            static_cast<VkDeviceSize>(desc.size),
                            usageFlags(desc.usage),
                            properties,
                            m_buffer,
                            m_memory))
    {
        m_context = nullptr;
        m_size = 0;
        return false;
    }

    return true;
}

void VulkanBuffer::shutdown()
{
    if (m_context == nullptr)
    {
        return;
    }

    unmap();
    destroyVulkanBuffer(*m_context, m_buffer, m_memory);
    m_context = nullptr;
    m_size = 0;
    m_hostVisible = false;
}

std::size_t VulkanBuffer::size() const
{
    return m_size;
}

bool VulkanBuffer::write(const void* data, std::size_t bytes, std::size_t offset)
{
    if (m_context == nullptr || data == nullptr || bytes == 0 ||
        offset + bytes > m_size)
    {
        return false;
    }

    if (m_hostVisible)
    {
        void* destination = m_mapped;

        if (destination == nullptr)
        {
            if (!checkResult(vkMapMemory(m_context->device(),
                                         m_memory,
                                         0,
                                         VK_WHOLE_SIZE,
                                         0,
                                         &destination),
                             "vkMapMemory"))
            {
                return false;
            }
        }

        std::memcpy(static_cast<std::uint8_t*>(destination) + offset, data, bytes);

        if (m_mapped == nullptr)
        {
            vkUnmapMemory(m_context->device(), m_memory);
        }

        return true;
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

    std::memcpy(stagingData, data, bytes);
    vkUnmapMemory(m_context->device(), stagingMemory);

    const bool copied = submitImmediate(*m_context, [&](VkCommandBuffer commandBuffer)
    {
        VkBufferCopy region{};
        region.srcOffset = 0;
        region.dstOffset = static_cast<VkDeviceSize>(offset);
        region.size = static_cast<VkDeviceSize>(bytes);
        vkCmdCopyBuffer(commandBuffer, staging, m_buffer, 1, &region);
        return true;
    });

    destroyVulkanBuffer(*m_context, staging, stagingMemory);
    return copied;
}

void* VulkanBuffer::map()
{
    if (m_context == nullptr || !m_hostVisible)
    {
        return nullptr;
    }

    if (m_mapped != nullptr)
    {
        return m_mapped;
    }

    if (!checkResult(vkMapMemory(m_context->device(), m_memory, 0, VK_WHOLE_SIZE, 0, &m_mapped),
                     "vkMapMemory"))
    {
        m_mapped = nullptr;
        return nullptr;
    }

    return m_mapped;
}

void VulkanBuffer::unmap()
{
    if (m_context != nullptr && m_mapped != nullptr)
    {
        vkUnmapMemory(m_context->device(), m_memory);
    }

    m_mapped = nullptr;
}

VkBuffer VulkanBuffer::handle() const noexcept
{
    return m_buffer;
}

}

#endif
