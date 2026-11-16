#if defined(SPY_RENDERER_VULKAN)

#include "vulkan/device/vulkan_device.hpp"

#include "core/logging/log_macros.hpp"

#include "vulkan/context/vulkan_render_context.hpp"
#include "vulkan/resources/vulkan_buffer.hpp"
#include "vulkan/resources/vulkan_texture.hpp"

namespace spy::vulkan
{

VulkanDevice::~VulkanDevice()
{
    shutdown();
}

bool VulkanDevice::initialize(const rhi::DeviceDesc& desc)
{
    if (m_initialized)
    {
        return true;
    }

    m_desc = desc;

    if (!m_context.initialize(desc))
    {
        m_context.shutdown();
        return false;
    }

    if (!createDescriptorResources())
    {
        m_context.shutdown();
        return false;
    }

    m_capabilities = m_context.capabilities();
    m_initialized = true;

    SPY_LOG_INFO("Vulkan device initialized: {}", m_capabilities.deviceName);
    return true;
}

bool VulkanDevice::createDescriptorResources()
{
    VkDescriptorSetLayoutBinding binding{};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &binding;

    if (!checkResult(vkCreateDescriptorSetLayout(m_context.device(),
                                                 &layoutInfo,
                                                 nullptr,
                                                 &m_textureSetLayout),
                     "vkCreateDescriptorSetLayout"))
    {
        m_textureSetLayout = VK_NULL_HANDLE;
        return false;
    }

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 64;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.maxSets = 64;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;

    if (!checkResult(vkCreateDescriptorPool(m_context.device(),
                                            &poolInfo,
                                            nullptr,
                                            &m_descriptorPool),
                     "vkCreateDescriptorPool"))
    {
        vkDestroyDescriptorSetLayout(m_context.device(), m_textureSetLayout, nullptr);
        m_textureSetLayout = VK_NULL_HANDLE;
        m_descriptorPool = VK_NULL_HANDLE;
        return false;
    }

    return true;
}

void VulkanDevice::destroyDescriptorResources()
{
    if (m_descriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(m_context.device(), m_descriptorPool, nullptr);
        m_descriptorPool = VK_NULL_HANDLE;
    }

    if (m_textureSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(m_context.device(), m_textureSetLayout, nullptr);
        m_textureSetLayout = VK_NULL_HANDLE;
    }
}

void VulkanDevice::shutdown()
{
    if (!m_initialized)
    {
        return;
    }

    m_context.waitIdle();
    destroyDescriptorResources();
    m_context.shutdown();
    m_initialized = false;
}

void VulkanDevice::waitIdle()
{
    if (m_initialized)
    {
        m_context.waitIdle();
    }
}

std::unique_ptr<rhi::RenderContext> VulkanDevice::createRenderContext(
    const rhi::NativeWindow& window,
    std::uint32_t width,
    std::uint32_t height)
{
    if (!m_initialized)
    {
        return nullptr;
    }

    if (!m_context.initializeSurface(window))
    {
        return nullptr;
    }

    auto renderContext = std::make_unique<VulkanRenderContext>(m_context,
                                                               m_desc,
                                                               m_textureSetLayout);

    if (!renderContext->initialize(window, width, height, m_desc.enableVsync))
    {
        return nullptr;
    }

    return renderContext;
}

std::unique_ptr<rhi::Buffer> VulkanDevice::createBuffer(const rhi::BufferDesc& desc)
{
    if (!m_initialized)
    {
        return nullptr;
    }

    auto buffer = std::make_unique<VulkanBuffer>();

    if (!buffer->initialize(m_context, desc))
    {
        return nullptr;
    }

    return buffer;
}

std::unique_ptr<rhi::Texture> VulkanDevice::createTexture(const rhi::TextureDesc& desc)
{
    if (!m_initialized)
    {
        return nullptr;
    }

    auto texture = std::make_unique<VulkanTexture>();

    if (!texture->initialize(m_context, desc, m_textureSetLayout, m_descriptorPool))
    {
        return nullptr;
    }

    return texture;
}

rhi::DeviceCapabilities VulkanDevice::capabilities() const
{
    return m_capabilities;
}

bool VulkanDevice::isValid() const
{
    return m_initialized && m_context.valid();
}

std::string_view VulkanDevice::backendName() const
{
    return "Vulkan";
}

}

#endif
