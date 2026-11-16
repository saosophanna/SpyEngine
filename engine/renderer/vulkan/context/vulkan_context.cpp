#if defined(SPY_RENDERER_VULKAN)

#include "vulkan/context/vulkan_context.hpp"

#include <cstring>
#include <string>
#include <string_view>
#include <vector>

#include "core/logging/log_macros.hpp"

namespace spy::vulkan
{

namespace
{

constexpr const char* kValidationLayer = "VK_LAYER_KHRONOS_validation";

bool validationLayerAvailable()
{
    std::uint32_t layerCount = 0;
    if (vkEnumerateInstanceLayerProperties(&layerCount, nullptr) != VK_SUCCESS)
    {
        return false;
    }

    std::vector<VkLayerProperties> layers(layerCount);
    if (vkEnumerateInstanceLayerProperties(&layerCount, layers.data()) != VK_SUCCESS)
    {
        return false;
    }

    for (const VkLayerProperties& layer : layers)
    {
        if (std::strcmp(layer.layerName, kValidationLayer) == 0)
        {
            return true;
        }
    }

    return false;
}

bool instanceExtensionAvailable(const char* name)
{
    std::uint32_t extensionCount = 0;
    if (vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr) != VK_SUCCESS)
    {
        return false;
    }

    std::vector<VkExtensionProperties> extensions(extensionCount);
    if (vkEnumerateInstanceExtensionProperties(
            nullptr, &extensionCount, extensions.data()) != VK_SUCCESS)
    {
        return false;
    }

    for (const VkExtensionProperties& extension : extensions)
    {
        if (std::strcmp(extension.extensionName, name) == 0)
        {
            return true;
        }
    }

    return false;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void*)
{
    const char* text = data != nullptr && data->pMessage != nullptr ? data->pMessage : "";

    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        SPY_LOG_ERROR("Vulkan validation: {}", text);
    }
    else if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        SPY_LOG_WARN("Vulkan validation: {}", text);
    }
    else
    {
        SPY_LOG_INFO("Vulkan validation: {}", text);
    }

    return VK_FALSE;
}

}

VulkanContext::~VulkanContext()
{
    shutdown();
}

bool VulkanContext::initialize(const rhi::DeviceDesc& desc)
{
    if (!createInstance(desc))
    {
        return false;
    }

    if (m_validationEnabled && !createDebugMessenger())
    {
        shutdown();
        return false;
    }

    if (!selectPhysicalDevice())
    {
        shutdown();
        return false;
    }

    if (!createLogicalDevice())
    {
        shutdown();
        return false;
    }

    fillCapabilities(desc);
    return true;
}

bool VulkanContext::initializeSurface(const rhi::NativeWindow& window)
{
    if (!valid())
    {
        return false;
    }

    if (m_surface != VK_NULL_HANDLE)
    {
        SPY_LOG_ERROR("Vulkan surface already exists for this device");
        return false;
    }

    if (!createSurface(window))
    {
        return false;
    }

    VkBool32 presentSupported = VK_FALSE;
    if (!checkResult(vkGetPhysicalDeviceSurfaceSupportKHR(
                         m_physicalDevice, m_queueFamilies.graphics, m_surface, &presentSupported),
                     "vkGetPhysicalDeviceSurfaceSupportKHR"))
    {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
        return false;
    }

    if (presentSupported != VK_TRUE)
    {
        SPY_LOG_ERROR("Vulkan graphics queue family cannot present to the surface");
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
        return false;
    }

    m_queueFamilies.present = m_queueFamilies.graphics;
    m_presentQueue = m_graphicsQueue;
    return true;
}

void VulkanContext::shutdown()
{
    if (m_device != VK_NULL_HANDLE)
    {
        vkDeviceWaitIdle(m_device);
        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }

    m_graphicsQueue = VK_NULL_HANDLE;
    m_presentQueue = VK_NULL_HANDLE;

    if (m_surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }

    if (m_debugMessenger != VK_NULL_HANDLE)
    {
        auto destroyMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT"));

        if (destroyMessenger != nullptr)
        {
            destroyMessenger(m_instance, m_debugMessenger, nullptr);
        }

        m_debugMessenger = VK_NULL_HANDLE;
    }

    if (m_instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }

    m_physicalDevice = VK_NULL_HANDLE;
    m_queueFamilies = QueueFamilyIndices{};
}

void VulkanContext::waitIdle()
{
    if (m_device != VK_NULL_HANDLE)
    {
        vkDeviceWaitIdle(m_device);
    }
}

bool VulkanContext::createInstance(const rhi::DeviceDesc& desc)
{
    const std::string applicationName(desc.applicationName);

    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = applicationName.c_str();
    applicationInfo.applicationVersion = desc.applicationVersion;
    applicationInfo.pEngineName = "SpyEngine";
    applicationInfo.engineVersion = 1;

    std::uint32_t loaderVersion = VK_API_VERSION_1_0;
    if (vkEnumerateInstanceVersion(&loaderVersion) != VK_SUCCESS)
    {
        loaderVersion = VK_API_VERSION_1_0;
    }

    applicationInfo.apiVersion =
        loaderVersion >= VK_API_VERSION_1_2 ? VK_API_VERSION_1_2 : VK_API_VERSION_1_0;

    std::vector<const char*> extensions;
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#if defined(VK_USE_PLATFORM_WIN32_KHR)
    extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else
    SPY_LOG_ERROR("Vulkan surface extension is unavailable on this platform");
    return false;
#endif

    for (const char* extension : extensions)
    {
        if (!instanceExtensionAvailable(extension))
        {
            SPY_LOG_ERROR("Required Vulkan instance extension {} is unavailable", extension);
            return false;
        }
    }

    m_validationEnabled = desc.enableValidation && validationLayerAvailable();

    if (desc.enableValidation && !m_validationEnabled)
    {
        SPY_LOG_WARN("Vulkan validation layer {} is unavailable", kValidationLayer);
    }

    if (m_validationEnabled && instanceExtensionAvailable(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    else
    {
        m_validationEnabled = false;
    }

    VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
    debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugInfo.pfnUserCallback = debugCallback;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &applicationInfo;
    createInfo.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    const char* layerNames[] = {kValidationLayer};

    if (m_validationEnabled)
    {
        createInfo.enabledLayerCount = 1;
        createInfo.ppEnabledLayerNames = layerNames;
        createInfo.pNext = &debugInfo;
    }

    if (!checkResult(vkCreateInstance(&createInfo, nullptr, &m_instance), "vkCreateInstance"))
    {
        m_instance = VK_NULL_HANDLE;
        return false;
    }

    return true;
}

bool VulkanContext::createDebugMessenger()
{
    auto createMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT"));

    if (createMessenger == nullptr)
    {
        return false;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;

    return checkResult(createMessenger(m_instance, &createInfo, nullptr, &m_debugMessenger),
                       "vkCreateDebugUtilsMessengerEXT");
}

bool VulkanContext::createSurface(const rhi::NativeWindow& window)
{
    if (!window.isValid())
    {
        SPY_LOG_ERROR("Vulkan surface requires a valid native window");
        return false;
    }

#if defined(VK_USE_PLATFORM_WIN32_KHR)
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = static_cast<HWND>(window.window);
    createInfo.hinstance = static_cast<HINSTANCE>(window.instance);

    if (!checkResult(vkCreateWin32SurfaceKHR(m_instance, &createInfo, nullptr, &m_surface),
                     "vkCreateWin32SurfaceKHR"))
    {
        m_surface = VK_NULL_HANDLE;
        return false;
    }

    return true;
#else
    (void)window;
    SPY_LOG_ERROR("Vulkan surface creation is not implemented for this platform");
    return false;
#endif
}

bool VulkanContext::selectPhysicalDevice()
{
    std::uint32_t deviceCount = 0;
    if (!checkResult(vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr),
                     "vkEnumeratePhysicalDevices"))
    {
        return false;
    }

    if (deviceCount == 0)
    {
        SPY_LOG_ERROR("No Vulkan physical devices found");
        return false;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    if (!checkResult(vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data()),
                     "vkEnumeratePhysicalDevices"))
    {
        return false;
    }

    VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
    std::int32_t bestScore = -1;

    for (VkPhysicalDevice device : devices)
    {
        if (!deviceExtensionsSupported(device))
        {
            continue;
        }

        if (!findQueueFamilies(device).hasGraphics())
        {
            continue;
        }

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(device, &properties);

        std::int32_t score = 1;

        switch (properties.deviceType)
        {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            score = 1000;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            score = 500;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            score = 300;
            break;
        default:
            score = 100;
            break;
        }

        if (score > bestScore)
        {
            bestScore = score;
            bestDevice = device;
        }
    }

    if (bestDevice == VK_NULL_HANDLE)
    {
        SPY_LOG_ERROR("No suitable Vulkan physical device found");
        return false;
    }

    m_physicalDevice = bestDevice;
    m_queueFamilies = findQueueFamilies(bestDevice);

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);
    SPY_LOG_INFO("Vulkan device selected: {}", properties.deviceName);

    return true;
}

bool VulkanContext::createLogicalDevice()
{
    if (!m_queueFamilies.hasGraphics())
    {
        return false;
    }

    const float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo queueInfo{};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = m_queueFamilies.graphics;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &queuePriority;

    const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueInfo;
    createInfo.enabledExtensionCount = 1;
    createInfo.ppEnabledExtensionNames = deviceExtensions;

    if (!checkResult(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device),
                     "vkCreateDevice"))
    {
        m_device = VK_NULL_HANDLE;
        return false;
    }

    vkGetDeviceQueue(m_device, m_queueFamilies.graphics, 0, &m_graphicsQueue);
    m_presentQueue = m_graphicsQueue;

    return true;
}

void VulkanContext::fillCapabilities(const rhi::DeviceDesc& desc)
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);

    VkPhysicalDeviceFeatures features{};
    vkGetPhysicalDeviceFeatures(m_physicalDevice, &features);

    m_capabilities.deviceName = properties.deviceName;
    m_capabilities.apiVersion = properties.apiVersion;
    m_capabilities.maxTextureSize = properties.limits.maxImageDimension2D;
    m_capabilities.maxFramesInFlight = desc.framesInFlight;
    m_capabilities.maxDescriptorSets = properties.limits.maxBoundDescriptorSets;
    m_capabilities.maxPushConstantsSize = properties.limits.maxPushConstantsSize;
    m_capabilities.supportsCompute = true;
    m_capabilities.supportsGeometry = features.geometryShader == VK_TRUE;
    m_capabilities.supportsSamplerAnisotropy = features.samplerAnisotropy == VK_TRUE;
}

QueueFamilyIndices VulkanContext::findQueueFamilies(VkPhysicalDevice device) const
{
    QueueFamilyIndices indices;

    std::uint32_t familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, nullptr);

    std::vector<VkQueueFamilyProperties> families(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, families.data());

    for (std::uint32_t index = 0; index < familyCount; ++index)
    {
        if ((families[index].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
        {
            indices.graphics = index;
            break;
        }
    }

    return indices;
}

bool VulkanContext::deviceExtensionsSupported(VkPhysicalDevice device) const
{
    std::uint32_t extensionCount = 0;
    if (vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr) !=
        VK_SUCCESS)
    {
        return false;
    }

    std::vector<VkExtensionProperties> extensions(extensionCount);
    if (vkEnumerateDeviceExtensionProperties(
            device, nullptr, &extensionCount, extensions.data()) != VK_SUCCESS)
    {
        return false;
    }

    for (const VkExtensionProperties& extension : extensions)
    {
        if (std::strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
        {
            return true;
        }
    }

    return false;
}

}

#endif
