#pragma once

#include <cstdint>
#include <memory>
#include <string_view>

namespace spy::rhi {

class Buffer;
class Texture;
class Pipeline;
class CommandBuffer;

enum class Backend {
    Vulkan,
    OpenGL,
};

struct DeviceDesc {
    Backend       backend          = Backend::Vulkan;
    const char*   applicationName  = "SpyEngine";
    bool          enableValidation = true;
    bool          enableVsync      = true;
    std::uint32_t framesInFlight   = 2;
};

class Device {
public:
    virtual ~Device() = default;

    Device(const Device&)            = delete;
    Device& operator=(const Device&) = delete;

    virtual bool initialize(const DeviceDesc& desc) = 0;
    virtual void shutdown() = 0;

    virtual void waitIdle() = 0;

    virtual std::unique_ptr<Buffer>        createBuffer()        = 0;
    virtual std::unique_ptr<Texture>       createTexture()       = 0;
    virtual std::unique_ptr<Pipeline>      createPipeline()      = 0;
    virtual std::unique_ptr<CommandBuffer> createCommandBuffer() = 0;

    [[nodiscard]] virtual bool isValid() const = 0;
    [[nodiscard]] virtual std::string_view backendName() const = 0;

protected:
    Device() = default;
};

}
