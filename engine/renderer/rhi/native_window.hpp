#pragma once

namespace spy::rhi
{

struct NativeWindow
{
    void* window = nullptr;
    void* display = nullptr;
    void* instance = nullptr;

    [[nodiscard]] bool isValid() const noexcept
    {
        return window != nullptr;
    }
};

}
