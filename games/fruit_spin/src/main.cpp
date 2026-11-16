#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string_view>

#include "core/logging/log_macros.hpp"
#include "core/time/clock.hpp"
#include "core/time/frame_limiter.hpp"
#include "platform/window/window.hpp"
#include "renderer/camera/camera2d.hpp"
#include "renderer/renderer.hpp"
#include "renderer/rhi/render_context.hpp"
#include "renderer/sprites/sprite_renderer.hpp"
#include "scenes/game_scene.hpp"

namespace
{

std::filesystem::path executableDirectory(const char* argument)
{
    std::error_code error;
    const std::filesystem::path path = std::filesystem::absolute(argument, error);

    if (error)
    {
        return std::filesystem::current_path(error);
    }

    return path.parent_path();
}

}

int main(int argc, char** argv)
{
    int frameLimit = 0;
    int maxFps = 0;

    for (int index = 1; index < argc; ++index)
    {
        const std::string_view argument(argv[index]);

        if (argument == "--frames" && index + 1 < argc)
        {
            frameLimit = std::atoi(argv[++index]);
        }
        else if (argument == "--max-fps" && index + 1 < argc)
        {
            maxFps = std::atoi(argv[++index]);
        }
    }

    spy::time::FrameLimiter frameLimiter;

    if (maxFps > 0)
    {
        frameLimiter.setTargetFps(static_cast<spy::uint32>(maxFps));
    }
    else
    {
        frameLimiter.setUnlimited();
    }

    const std::filesystem::path root = executableDirectory(argv[0]);

    spy::platform::Window window;
    spy::platform::WindowDesc windowDesc{};
    windowDesc.width = 1280;
    windowDesc.height = 720;
    windowDesc.title = "Fruit Spin";
    windowDesc.resizable = true;

    if (!window.create(windowDesc))
    {
        SPY_LOG_ERROR("fruit_spin: window creation failed");
        return 1;
    }

    std::unique_ptr<spy::rhi::Device> device =
        spy::renderer::createDevice(spy::rhi::Backend::Vulkan);

    if (device == nullptr)
    {
        SPY_LOG_ERROR("fruit_spin: Vulkan backend unavailable");
        return 1;
    }

    spy::rhi::DeviceDesc deviceDesc{};
    deviceDesc.backend = spy::rhi::Backend::Vulkan;
    deviceDesc.applicationName = "Fruit Spin";
    deviceDesc.applicationVersion = 1;
    deviceDesc.enableValidation = true;
    deviceDesc.enableVsync = true;
    deviceDesc.framesInFlight = 2;

    if (!device->initialize(deviceDesc))
    {
        SPY_LOG_ERROR("fruit_spin: device initialization failed");
        return 1;
    }

    spy::rhi::NativeWindow nativeWindow{};
    nativeWindow.window = window.nativeHandle();
    nativeWindow.instance = window.instanceHandle();

    std::unique_ptr<spy::rhi::RenderContext> renderContext =
        device->createRenderContext(nativeWindow, window.width(), window.height());

    if (renderContext == nullptr)
    {
        SPY_LOG_ERROR("fruit_spin: render context creation failed");
        device->shutdown();
        return 1;
    }

    const std::filesystem::path shaderDirectory = root / "shaders" / "2d";

    spy::renderer::SpriteRenderer spriteRenderer;

    if (!spriteRenderer.initialize(*device,
                                   *renderContext,
                                   (shaderDirectory / "sprite.vert.spv").string(),
                                   (shaderDirectory / "sprite.frag.spv").string()))
    {
        SPY_LOG_ERROR("fruit_spin: sprite renderer initialization failed");
        device->shutdown();
        return 1;
    }

    fruit_spin::GameScene scene;

    if (!scene.initialize(spriteRenderer, (root / "assets").string()))
    {
        SPY_LOG_ERROR("fruit_spin: game initialization failed");
        spriteRenderer.shutdown();
        device->shutdown();
        return 1;
    }

    spy::renderer::Camera2D camera(static_cast<float>(window.width()),
                                   static_cast<float>(window.height()));

    double previous = spy::time::Clock::nowSeconds();
    int frameCount = 0;
    int exitCode = 0;
    bool running = true;

    const float deltaCeiling =
        maxFps > 0 ? std::max(0.05f, 1.25f / static_cast<float>(maxFps)) : 0.05f;

    while (running)
    {
        frameLimiter.beginFrame();

        if (!window.pollEvents())
        {
            break;
        }

        if (window.shouldClose() || window.wasKeyPressed(spy::platform::Key::Escape))
        {
            break;
        }

        const std::uint32_t width = window.width();
        const std::uint32_t height = window.height();

        if (width == 0 || height == 0)
        {
            frameLimiter.endFrame();
            continue;
        }

        if (width != renderContext->width() || height != renderContext->height())
        {
            if (!renderContext->resize(width, height))
            {
                exitCode = 1;
                break;
            }

            camera.resize(static_cast<float>(width), static_cast<float>(height));
        }

        const double now = spy::time::Clock::nowSeconds();
        float deltaTime = static_cast<float>(now - previous);
        previous = now;
        deltaTime = std::clamp(deltaTime, 0.0f, deltaCeiling);

        scene.update(deltaTime, window);

        if (renderContext->beginFrame())
        {
            spy::rhi::CommandBuffer& commands = renderContext->currentCommandBuffer();
            commands.beginRenderPass(spy::rhi::Color{0.025f, 0.018f, 0.055f, 1.0f});

            spriteRenderer.begin();
            spriteRenderer.setProjection(camera.projection());
            scene.render(spriteRenderer,
                         static_cast<float>(width),
                         static_cast<float>(height));
            spriteRenderer.end(commands);

            if (!renderContext->endFrame())
            {
                exitCode = 1;
                break;
            }
        }

        ++frameCount;

        if (frameLimit > 0 && frameCount >= frameLimit)
        {
            running = false;
        }

        frameLimiter.endFrame();
    }

    renderContext->waitIdle();
    spriteRenderer.shutdown();
    renderContext.reset();
    device->waitIdle();
    device->shutdown();
    window.destroy();

    return exitCode;
}
