#include <cstdio>
#include <memory>

#include "renderer/renderer.hpp"
#include "rhi/rhi.hpp"

#if defined(SPY_PLATFORM_WINDOWS) && defined(SPY_RENDERER_VULKAN)
#include <windows.h>

#if defined(DeviceCapabilities)
#undef DeviceCapabilities
#endif
#endif

namespace {

int g_failures = 0;

void check(bool condition, const char* expression)
{
    if (!condition)
    {
        std::fprintf(stderr, "FAILED: %s\n", expression);
        ++g_failures;
    }
}

#if defined(SPY_PLATFORM_WINDOWS) && defined(SPY_RENDERER_VULKAN)
bool runSwapchainChecks(spy::rhi::Device& device)
{
    const HINSTANCE instance = GetModuleHandleW(nullptr);
    const wchar_t* className = L"SpyEngineRendererTestWindow";

    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.lpfnWndProc = DefWindowProcW;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = className;

    if (RegisterClassExW(&windowClass) == 0)
    {
        std::puts("window class registration failed; skipping swapchain checks");
        return true;
    }

    const HWND windowHandle = CreateWindowExW(0,
                                              className,
                                              L"SpyEngine",
                                              WS_OVERLAPPEDWINDOW,
                                              CW_USEDEFAULT,
                                              CW_USEDEFAULT,
                                              320,
                                              240,
                                              nullptr,
                                              nullptr,
                                              instance,
                                              nullptr);

    if (windowHandle == nullptr)
    {
        UnregisterClassW(className, instance);
        std::puts("window creation failed; skipping swapchain checks");
        return true;
    }

    bool passed = true;

    {
        spy::rhi::NativeWindow window{};
        window.window = windowHandle;
        window.instance = instance;

        std::unique_ptr<spy::rhi::RenderContext> context =
            device.createRenderContext(window, 320, 240);

        if (context == nullptr)
        {
            DestroyWindow(windowHandle);
            UnregisterClassW(className, instance);
            std::puts("swapchain creation failed; skipping swapchain checks");
            return true;
        }

        for (int frame = 0; frame < 3; ++frame)
        {
            if (!context->beginFrame())
            {
                std::puts("frame acquisition failed");
                passed = false;
                break;
            }

            context->currentCommandBuffer().beginRenderPass(
                spy::rhi::Color{0.05f, 0.08f, 0.12f, 1.0f});

            if (!context->endFrame())
            {
                std::puts("frame submission failed");
                passed = false;
                break;
            }
        }

        if (passed)
        {
            check(context->swapchain().imageCount() >= 2, "swapchain provides back buffers");
            check(context->swapchain().width() > 0, "swapchain width is positive");
            check(context->swapchain().height() > 0, "swapchain height is positive");
            check(context->currentFrameIndex() < 2, "frame index wraps over frames in flight");
        }

        context->waitIdle();
    }

    DestroyWindow(windowHandle);
    UnregisterClassW(className, instance);
    return passed;
}
#endif

}

int main()
{
    const spy::rhi::NativeWindow window{};
    check(!window.isValid(), "default native window is invalid");

    const spy::rhi::DeviceDesc desc{};
    check(desc.backend == spy::rhi::Backend::Vulkan, "default backend is Vulkan");
    check(desc.framesInFlight == 2, "default frames in flight is two");
    check(desc.enableValidation, "validation is enabled by default");

    const auto openGlDevice = spy::renderer::createDevice(spy::rhi::Backend::OpenGL);
    check(openGlDevice == nullptr, "OpenGL backend is not built");

    auto device = spy::renderer::createDevice(spy::rhi::Backend::Vulkan);

    if (device == nullptr)
    {
        std::puts("Vulkan backend not built; skipping device checks");
        return g_failures == 0 ? 0 : 1;
    }

    spy::rhi::DeviceDesc deviceDesc{};

    if (!device->initialize(deviceDesc))
    {
        std::puts("Vulkan device unavailable on this machine; skipping device checks");
        return g_failures == 0 ? 0 : 1;
    }

    check(device->isValid(), "device reports valid after initialization");
    check(device->backendName() == "Vulkan", "backend name is Vulkan");

    const spy::rhi::DeviceCapabilities capabilities = device->capabilities();
    check(!capabilities.deviceName.empty(), "device name is populated");
    check(capabilities.maxTextureSize >= 1024, "device supports reasonable texture sizes");
    check(capabilities.maxFramesInFlight == 2, "capabilities report frames in flight");
    check(capabilities.maxDescriptorSets > 0, "device supports descriptor sets");

#if defined(SPY_PLATFORM_WINDOWS) && defined(SPY_RENDERER_VULKAN)
    if (!runSwapchainChecks(*device))
    {
        ++g_failures;
    }
#endif

    device->waitIdle();
    device->shutdown();
    check(!device->isValid(), "device reports invalid after shutdown");

    std::printf("%s\n", g_failures == 0 ? "renderer tests passed" : "renderer tests failed");
    return g_failures == 0 ? 0 : 1;
}
