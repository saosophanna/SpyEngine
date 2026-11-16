#pragma once

#include <cstdint>
#include <functional>

#include "vulkan/vulkan_common.hpp"

#if defined(SPY_RENDERER_VULKAN)

namespace spy::vulkan
{

class VulkanContext;

[[nodiscard]] std::uint32_t findMemoryType(VulkanContext& context,
                                           std::uint32_t typeBits,
                                           VkMemoryPropertyFlags properties);

[[nodiscard]] bool createVulkanBuffer(VulkanContext& context,
                                      VkDeviceSize size,
                                      VkBufferUsageFlags usage,
                                      VkMemoryPropertyFlags properties,
                                      VkBuffer& buffer,
                                      VkDeviceMemory& memory);

void destroyVulkanBuffer(VulkanContext& context, VkBuffer& buffer, VkDeviceMemory& memory);

[[nodiscard]] bool submitImmediate(VulkanContext& context,
                                   const std::function<bool(VkCommandBuffer)>& record);

[[nodiscard]] bool transitionImageLayout(VkCommandBuffer commandBuffer,
                                         VkImage image,
                                         VkImageLayout oldLayout,
                                         VkImageLayout newLayout);

[[nodiscard]] bool copyBufferToImage(VkCommandBuffer commandBuffer,
                                     VkBuffer buffer,
                                     VkImage image,
                                     std::uint32_t width,
                                     std::uint32_t height);

}

#endif
