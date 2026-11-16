#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include "rhi/device_capabilities.hpp"
#include "rhi/native_window.hpp"

namespace spy::rhi
{

class RenderContext;

enum class Backend
{
    Vulkan,
    OpenGL,
};

struct DeviceDesc
{
    Backend backend = Backend::Vulkan;
    std::string applicationName = "SpyEngine";
    std::uint32_t applicationVersion = 1;
    bool enableValidation = true;
    bool enableVsync = true;
    std::uint32_t framesInFlight = 2;
};

class Device
{
public:
    virtual ~Device() = default;

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    [[nodiscard]] virtual bool initialize(const DeviceDesc& desc) = 0;
    virtual void shutdown() = 0;
    virtual void waitIdle() = 0;

    [[nodiscard]] virtual std::unique_ptr<RenderContext> createRenderContext(
        const NativeWindow& window,
        std::uint32_t width,
        std::uint32_t height) = 0;

    [[nodiscard]] virtual DeviceCapabilities capabilities() const = 0;
    [[nodiscard]] virtual bool isValid() const = 0;
    [[nodiscard]] virtual std::string_view backendName() const = 0;

protected:
    Device() = default;
};

}
