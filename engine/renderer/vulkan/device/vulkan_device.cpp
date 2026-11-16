#if defined(SPY_RENDERER_VULKAN)

#include "vulkan/device/vulkan_device.hpp"

#include "core/logging/log_macros.hpp"

#include "vulkan/context/vulkan_render_context.hpp"

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

    m_capabilities = m_context.capabilities();
    m_initialized = true;

    SPY_LOG_INFO("Vulkan device initialized: {}", m_capabilities.deviceName);
    return true;
}

void VulkanDevice::shutdown()
{
    if (!m_initialized)
    {
        return;
    }

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

    auto renderContext = std::make_unique<VulkanRenderContext>(m_context, m_desc);

    if (!renderContext->initialize(window, width, height, m_desc.enableVsync))
    {
        return nullptr;
    }

    return renderContext;
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
