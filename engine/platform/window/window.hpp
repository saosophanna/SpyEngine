#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

#include "platform/window/key.hpp"

namespace spy::platform
{

struct WindowMessageHandler;

struct WindowDesc
{
    std::uint32_t width = 1280;
    std::uint32_t height = 720;
    std::string title = "SpyEngine";
    bool resizable = true;
};

class Window
{
public:
    Window() = default;
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    [[nodiscard]] bool create(const WindowDesc& desc);
    void destroy();

    [[nodiscard]] bool pollEvents();
    [[nodiscard]] bool shouldClose() const noexcept;

    [[nodiscard]] std::uint32_t width() const noexcept;
    [[nodiscard]] std::uint32_t height() const noexcept;

    [[nodiscard]] void* nativeHandle() const noexcept;
    [[nodiscard]] void* instanceHandle() const noexcept;

    [[nodiscard]] bool isKeyDown(Key key) const noexcept;
    [[nodiscard]] bool wasKeyPressed(Key key) const noexcept;

    [[nodiscard]] float mouseX() const noexcept;
    [[nodiscard]] float mouseY() const noexcept;
    [[nodiscard]] bool isMouseDown() const noexcept;
    [[nodiscard]] bool wasMousePressed() const noexcept;

    void setTitle(std::string_view title);

private:
    friend struct WindowMessageHandler;

    void onKey(bool down, std::uint32_t virtualKey);
    void onResize(std::uint32_t width, std::uint32_t height);
    void onMouseMove(float x, float y);
    void onMouseButton(bool down, bool pressed);

    void* m_window = nullptr;
    void* m_instance = nullptr;
    std::uint32_t m_width = 0;
    std::uint32_t m_height = 0;
    std::array<bool, kKeyCount> m_keyDown{};
    std::array<bool, kKeyCount> m_keyPressed{};
    float m_mouseX = 0.0f;
    float m_mouseY = 0.0f;
    bool m_mouseDown = false;
    bool m_mousePressed = false;
    bool m_shouldClose = false;
};

}
