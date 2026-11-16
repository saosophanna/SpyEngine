#include "platform/window/window.hpp"

#if defined(SPY_PLATFORM_WINDOWS)

#include <windows.h>

#include <utility>

namespace spy::platform
{

struct WindowMessageHandler
{
    static std::intptr_t dispatch(Window& window,
                                  void* nativeWindow,
                                  unsigned int message,
                                  std::uintptr_t wParam,
                                  std::intptr_t lParam);
};

namespace
{

constexpr wchar_t kWindowClassName[] = L"SpyEngineWindow";

LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

bool registerWindowClass(HINSTANCE instance)
{
    static bool registered = false;

    if (registered)
    {
        return true;
    }

    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc = windowProcedure;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.lpszClassName = kWindowClassName;

    if (RegisterClassExW(&windowClass) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
    {
        return false;
    }

    registered = true;
    return true;
}

std::wstring widen(std::string_view text)
{
    if (text.empty())
    {
        return std::wstring{};
    }

    const int length = MultiByteToWideChar(CP_UTF8,
                                           0,
                                           text.data(),
                                           static_cast<int>(text.size()),
                                           nullptr,
                                           0);

    if (length <= 0)
    {
        return std::wstring{};
    }

    std::wstring result(static_cast<std::size_t>(length), L'\0');
    MultiByteToWideChar(CP_UTF8,
                        0,
                        text.data(),
                        static_cast<int>(text.size()),
                        result.data(),
                        length);
    return result;
}

Key keyFromVirtualKey(std::uintptr_t key)
{
    switch (key)
    {
        case VK_ESCAPE:
            return Key::Escape;
        case VK_RETURN:
            return Key::Enter;
        case VK_SPACE:
            return Key::Space;
        case VK_LEFT:
            return Key::Left;
        case VK_RIGHT:
            return Key::Right;
        case VK_UP:
            return Key::Up;
        case VK_DOWN:
            return Key::Down;
        case VK_HOME:
            return Key::Home;
        case VK_END:
            return Key::End;
        case VK_PRIOR:
            return Key::PageUp;
        case VK_NEXT:
            return Key::PageDown;
        case 'A':
            return Key::A;
        case 'D':
            return Key::D;
        case 'R':
            return Key::R;
        case 'S':
            return Key::S;
        case 'W':
            return Key::W;
        default:
            return Key::Unknown;
    }
}

std::intptr_t windowProcedureResult(HWND window,
                                    unsigned int message,
                                    std::uintptr_t wParam,
                                    std::intptr_t lParam)
{
    return static_cast<std::intptr_t>(
        DefWindowProcW(window, message, static_cast<WPARAM>(wParam), static_cast<LPARAM>(lParam)));
}

}

std::intptr_t WindowMessageHandler::dispatch(Window& window,
                                             void* nativeWindow,
                                             unsigned int message,
                                             std::uintptr_t wParam,
                                             std::intptr_t lParam)
{
    HWND handle = static_cast<HWND>(nativeWindow);

    switch (message)
    {
        case WM_CLOSE:
            window.m_shouldClose = true;
            DestroyWindow(handle);
            return 0;
        case WM_DESTROY:
            window.m_shouldClose = true;
            window.m_window = nullptr;
            return 0;
        case WM_ERASEBKGND:
            return 1;
        case WM_SIZE:
        {
            if (wParam == SIZE_MINIMIZED)
            {
                return 0;
            }

            const std::uint32_t newWidth = static_cast<std::uint32_t>(LOWORD(lParam));
            const std::uint32_t newHeight = static_cast<std::uint32_t>(HIWORD(lParam));
            window.onResize(newWidth, newHeight);
            return 0;
        }
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            window.onKey(true, static_cast<std::uint32_t>(wParam));
            return 0;
        case WM_KEYUP:
        case WM_SYSKEYUP:
            window.onKey(false, static_cast<std::uint32_t>(wParam));
            return 0;
        case WM_MOUSEMOVE:
            window.onMouseMove(static_cast<float>(static_cast<std::int16_t>(LOWORD(lParam))),
                               static_cast<float>(static_cast<std::int16_t>(HIWORD(lParam))));
            return 0;
        case WM_LBUTTONDOWN:
            SetCapture(handle);
            window.onMouseButton(true, true);
            return 0;
        case WM_LBUTTONUP:
            ReleaseCapture();
            window.onMouseButton(false, false);
            return 0;
        default:
            break;
    }

    return windowProcedureResult(handle, message, wParam, lParam);
}

namespace
{

LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto* receiver = reinterpret_cast<Window*>(GetWindowLongPtrW(window, GWLP_USERDATA));

    if (message == WM_NCCREATE)
    {
        auto* create = reinterpret_cast<CREATESTRUCTW*>(lParam);
        receiver = static_cast<Window*>(create->lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(receiver));
    }

    if (receiver == nullptr)
    {
        return DefWindowProcW(window, message, wParam, lParam);
    }

    return static_cast<LRESULT>(WindowMessageHandler::dispatch(*receiver,
                                                               window,
                                                               message,
                                                               static_cast<std::uintptr_t>(wParam),
                                                               static_cast<std::intptr_t>(lParam)));
}

}

Window::~Window()
{
    destroy();
}

bool Window::create(const WindowDesc& desc)
{
    if (m_window != nullptr)
    {
        return true;
    }

    m_instance = GetModuleHandleW(nullptr);

    if (m_instance == nullptr)
    {
        return false;
    }

    if (!registerWindowClass(static_cast<HINSTANCE>(m_instance)))
    {
        return false;
    }

    m_width = desc.width == 0 ? 1280 : desc.width;
    m_height = desc.height == 0 ? 720 : desc.height;

    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

    if (desc.resizable)
    {
        style = WS_OVERLAPPEDWINDOW;
    }

    RECT bounds{0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height)};
    AdjustWindowRect(&bounds, style, FALSE);

    const std::wstring title = widen(desc.title);

    const HWND window = CreateWindowExW(0,
                                        kWindowClassName,
                                        title.c_str(),
                                        style,
                                        CW_USEDEFAULT,
                                        CW_USEDEFAULT,
                                        bounds.right - bounds.left,
                                        bounds.bottom - bounds.top,
                                        nullptr,
                                        nullptr,
                                        static_cast<HINSTANCE>(m_instance),
                                        this);

    if (window == nullptr)
    {
        m_instance = nullptr;
        return false;
    }

    m_window = window;
    ShowWindow(window, SW_SHOW);
    UpdateWindow(window);
    return true;
}

void Window::destroy()
{
    if (m_window != nullptr)
    {
        DestroyWindow(static_cast<HWND>(m_window));
        m_window = nullptr;
    }

    m_instance = nullptr;
}

bool Window::pollEvents()
{
    m_keyPressed.fill(false);
    m_mousePressed = false;

    MSG message{};

    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE) != 0)
    {
        if (message.message == WM_QUIT)
        {
            m_shouldClose = true;
            break;
        }

        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return !m_shouldClose;
}

bool Window::shouldClose() const noexcept
{
    return m_shouldClose;
}

std::uint32_t Window::width() const noexcept
{
    return m_width;
}

std::uint32_t Window::height() const noexcept
{
    return m_height;
}

void* Window::nativeHandle() const noexcept
{
    return m_window;
}

void* Window::instanceHandle() const noexcept
{
    return m_instance;
}

bool Window::isKeyDown(Key key) const noexcept
{
    const auto index = static_cast<std::size_t>(key);
    return index < m_keyDown.size() && m_keyDown[index];
}

bool Window::wasKeyPressed(Key key) const noexcept
{
    const auto index = static_cast<std::size_t>(key);
    return index < m_keyPressed.size() && m_keyPressed[index];
}

float Window::mouseX() const noexcept
{
    return m_mouseX;
}

float Window::mouseY() const noexcept
{
    return m_mouseY;
}

bool Window::isMouseDown() const noexcept
{
    return m_mouseDown;
}

bool Window::wasMousePressed() const noexcept
{
    return m_mousePressed;
}

void Window::setTitle(std::string_view title)
{
    if (m_window == nullptr)
    {
        return;
    }

    const std::wstring wideTitle = widen(title);
    SetWindowTextW(static_cast<HWND>(m_window), wideTitle.c_str());
}

void Window::onKey(bool down, std::uint32_t virtualKey)
{
    const Key key = keyFromVirtualKey(static_cast<std::uintptr_t>(virtualKey));

    if (key == Key::Unknown)
    {
        return;
    }

    const auto index = static_cast<std::size_t>(key);

    if (down)
    {
        if (!m_keyDown[index])
        {
            m_keyPressed[index] = true;
        }

        m_keyDown[index] = true;
        return;
    }

    m_keyDown[index] = false;
}

void Window::onResize(std::uint32_t width, std::uint32_t height)
{
    m_width = width;
    m_height = height;
}

void Window::onMouseMove(float x, float y)
{
    m_mouseX = x;
    m_mouseY = y;
}

void Window::onMouseButton(bool down, bool pressed)
{
    m_mouseDown = down;

    if (pressed)
    {
        m_mousePressed = true;
    }
}

}

#else

namespace spy::platform
{

struct WindowMessageHandler
{
};

Window::~Window() = default;

bool Window::create(const WindowDesc&)
{
    return false;
}

void Window::destroy()
{
}

bool Window::pollEvents()
{
    return false;
}

bool Window::shouldClose() const noexcept
{
    return true;
}

std::uint32_t Window::width() const noexcept
{
    return m_width;
}

std::uint32_t Window::height() const noexcept
{
    return m_height;
}

void* Window::nativeHandle() const noexcept
{
    return nullptr;
}

void* Window::instanceHandle() const noexcept
{
    return nullptr;
}

bool Window::isKeyDown(Key) const noexcept
{
    return false;
}

bool Window::wasKeyPressed(Key) const noexcept
{
    return false;
}

float Window::mouseX() const noexcept
{
    return m_mouseX;
}

float Window::mouseY() const noexcept
{
    return m_mouseY;
}

bool Window::isMouseDown() const noexcept
{
    return m_mouseDown;
}

bool Window::wasMousePressed() const noexcept
{
    return m_mousePressed;
}

void Window::setTitle(std::string_view)
{
}

void Window::onKey(bool, std::uint32_t)
{
}

void Window::onResize(std::uint32_t width, std::uint32_t height)
{
    m_width = width;
    m_height = height;
}

void Window::onMouseMove(float x, float y)
{
    m_mouseX = x;
    m_mouseY = y;
}

void Window::onMouseButton(bool down, bool pressed)
{
    m_mouseDown = down;

    if (pressed)
    {
        m_mousePressed = true;
    }
}

}

#endif
