#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string_view>

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

    for (int index = 1; index < argc; ++index)
    {
        const std::string_view argument(argv[index]);

        if (argument == "--frames" && index + 1 < argc)
        {
            frameLimit = std::atoi(argv[++index]);
        }
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
        std::fprintf(stderr, "fruit_spin: window creation failed\n");
        return 1;
    }

    std::unique_ptr<spy::rhi::Device> device =
        spy::renderer::createDevice(spy::rhi::Backend::Vulkan);

    if (device == nullptr)
    {
        std::fprintf(stderr, "fruit_spin: Vulkan backend unavailable\n");
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
        std::fprintf(stderr, "fruit_spin: device initialization failed\n");
        return 1;
    }

    spy::rhi::NativeWindow nativeWindow{};
    nativeWindow.window = window.nativeHandle();
    nativeWindow.instance = window.instanceHandle();

    std::unique_ptr<spy::rhi::RenderContext> renderContext =
        device->createRenderContext(nativeWindow, window.width(), window.height());

    if (renderContext == nullptr)
    {
        std::fprintf(stderr, "fruit_spin: render context creation failed\n");
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
        std::fprintf(stderr, "fruit_spin: sprite renderer initialization failed\n");
        device->shutdown();
        return 1;
    }

    fruit_spin::GameScene scene;

    if (!scene.initialize(spriteRenderer, (root / "assets").string()))
    {
        std::fprintf(stderr, "fruit_spin: game initialization failed\n");
        spriteRenderer.shutdown();
        device->shutdown();
        return 1;
    }

    spy::renderer::Camera2D camera(static_cast<float>(window.width()),
                                   static_cast<float>(window.height()));

    auto previous = std::chrono::steady_clock::now();
    int frameCount = 0;
    int exitCode = 0;
    bool running = true;

    while (running)
    {
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

        const auto now = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(now - previous).count();
        previous = now;
        deltaTime = std::clamp(deltaTime, 0.0f, 0.05f);

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
    }

    renderContext->waitIdle();
    spriteRenderer.shutdown();
    renderContext.reset();
    device->waitIdle();
    device->shutdown();
    window.destroy();

    return exitCode;
}
