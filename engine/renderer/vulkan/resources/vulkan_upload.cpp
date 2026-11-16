#if defined(SPY_RENDERER_VULKAN)

#include "vulkan/resources/vulkan_upload.hpp"

#include <cstring>

#include "vulkan/context/vulkan_context.hpp"

namespace spy::vulkan
{

std::uint32_t findMemoryType(VulkanContext& context,
                             std::uint32_t typeBits,
                             VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memoryProperties{};
    vkGetPhysicalDeviceMemoryProperties(context.physicalDevice(), &memoryProperties);

    for (std::uint32_t index = 0; index < memoryProperties.memoryTypeCount; ++index)
    {
        const bool supported = (typeBits & (1u << index)) != 0u;
        const bool matches =
            (memoryProperties.memoryTypes[index].propertyFlags & properties) == properties;

        if (supported && matches)
        {
            return index;
        }
    }

    return 0xFFFFFFFFu;
}

bool createVulkanBuffer(VulkanContext& context,
                        VkDeviceSize size,
                        VkBufferUsageFlags usage,
                        VkMemoryPropertyFlags properties,
                        VkBuffer& buffer,
                        VkDeviceMemory& memory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (!checkResult(vkCreateBuffer(context.device(), &bufferInfo, nullptr, &buffer),
                     "vkCreateBuffer"))
    {
        buffer = VK_NULL_HANDLE;
        return false;
    }

    VkMemoryRequirements requirements{};
    vkGetBufferMemoryRequirements(context.device(), buffer, &requirements);

    const std::uint32_t memoryType = findMemoryType(context, requirements.memoryTypeBits,
                                                    properties);

    if (memoryType == 0xFFFFFFFFu)
    {
        vkDestroyBuffer(context.device(), buffer, nullptr);
        buffer = VK_NULL_HANDLE;
        return false;
    }

    VkMemoryAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = requirements.size;
    allocateInfo.memoryTypeIndex = memoryType;

    if (!checkResult(vkAllocateMemory(context.device(), &allocateInfo, nullptr, &memory),
                     "vkAllocateMemory"))
    {
        vkDestroyBuffer(context.device(), buffer, nullptr);
        buffer = VK_NULL_HANDLE;
        memory = VK_NULL_HANDLE;
        return false;
    }

    if (!checkResult(vkBindBufferMemory(context.device(), buffer, memory, 0),
                     "vkBindBufferMemory"))
    {
        vkDestroyBuffer(context.device(), buffer, nullptr);
        vkFreeMemory(context.device(), memory, nullptr);
        buffer = VK_NULL_HANDLE;
        memory = VK_NULL_HANDLE;
        return false;
    }

    return true;
}

void destroyVulkanBuffer(VulkanContext& context, VkBuffer& buffer, VkDeviceMemory& memory)
{
    if (buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(context.device(), buffer, nullptr);
        buffer = VK_NULL_HANDLE;
    }

    if (memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(context.device(), memory, nullptr);
        memory = VK_NULL_HANDLE;
    }
}

bool submitImmediate(VulkanContext& context, const std::function<bool(VkCommandBuffer)>& record)
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = context.queueFamilies().graphics;

    VkCommandPool pool = VK_NULL_HANDLE;

    if (!checkResult(vkCreateCommandPool(context.device(), &poolInfo, nullptr, &pool),
                     "vkCreateCommandPool"))
    {
        return false;
    }

    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = pool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

    if (!checkResult(vkAllocateCommandBuffers(context.device(), &allocateInfo, &commandBuffer),
                     "vkAllocateCommandBuffers"))
    {
        vkDestroyCommandPool(context.device(), pool, nullptr);
        return false;
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (!checkResult(vkBeginCommandBuffer(commandBuffer, &beginInfo), "vkBeginCommandBuffer"))
    {
        vkDestroyCommandPool(context.device(), pool, nullptr);
        return false;
    }

    if (!record(commandBuffer))
    {
        vkDestroyCommandPool(context.device(), pool, nullptr);
        return false;
    }

    if (!checkResult(vkEndCommandBuffer(commandBuffer), "vkEndCommandBuffer"))
    {
        vkDestroyCommandPool(context.device(), pool, nullptr);
        return false;
    }

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    VkFence fence = VK_NULL_HANDLE;

    if (!checkResult(vkCreateFence(context.device(), &fenceInfo, nullptr, &fence),
                     "vkCreateFence"))
    {
        vkDestroyCommandPool(context.device(), pool, nullptr);
        return false;
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    const bool submitted =
        checkResult(vkQueueSubmit(context.graphicsQueue(), 1, &submitInfo, fence),
                    "vkQueueSubmit") &&
        checkResult(vkWaitForFences(context.device(), 1, &fence, VK_TRUE, UINT64_MAX),
                    "vkWaitForFences");

    vkDestroyFence(context.device(), fence, nullptr);
    vkDestroyCommandPool(context.device(), pool, nullptr);
    return submitted;
}

namespace
{

struct ImageTransition
{
    VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkAccessFlags sourceAccess = VK_ACCESS_MEMORY_WRITE_BIT;
    VkAccessFlags destinationAccess = VK_ACCESS_MEMORY_WRITE_BIT;
};

ImageTransition transitionFor(VkImageLayout oldLayout, VkImageLayout newLayout)
{
    ImageTransition transition{};

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        transition.sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        transition.destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        transition.sourceAccess = 0;
        transition.destinationAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
        return transition;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        transition.sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        transition.destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        transition.sourceAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
        transition.destinationAccess = VK_ACCESS_SHADER_READ_BIT;
        return transition;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        transition.sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        transition.destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        transition.sourceAccess = VK_ACCESS_SHADER_READ_BIT;
        transition.destinationAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
        return transition;
    }

    return transition;
}

}

bool transitionImageLayout(VkCommandBuffer commandBuffer,
                           VkImage image,
                           VkImageLayout oldLayout,
                           VkImageLayout newLayout)
{
    const ImageTransition transition = transitionFor(oldLayout, newLayout);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = transition.sourceAccess;
    barrier.dstAccessMask = transition.destinationAccess;

    vkCmdPipelineBarrier(commandBuffer,
                         transition.sourceStage,
                         transition.destinationStage,
                         0,
                         0,
                         nullptr,
                         0,
                         nullptr,
                         1,
                         &barrier);
    return true;
}

bool copyBufferToImage(VkCommandBuffer commandBuffer,
                       VkBuffer buffer,
                       VkImage image,
                       std::uint32_t width,
                       std::uint32_t height)
{
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(commandBuffer,
                           buffer,
                           image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &region);
    return true;
}

}

#endif
