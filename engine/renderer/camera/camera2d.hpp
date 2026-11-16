#pragma once

#include "core/math/matrix/mat4.hpp"

namespace spy::renderer
{

class Camera2D
{
public:
    Camera2D() = default;
    explicit Camera2D(float width, float height);

    void resize(float width, float height);

    [[nodiscard]] const math::Mat4& projection() const noexcept;
    [[nodiscard]] float width() const noexcept;
    [[nodiscard]] float height() const noexcept;

private:
    void rebuild();

    math::Mat4 m_projection{};
    float m_width = 0.0f;
    float m_height = 0.0f;
};

}
