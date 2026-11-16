#include "camera/camera2d.hpp"

namespace spy::renderer
{

Camera2D::Camera2D(float width, float height)
{
    resize(width, height);
}

void Camera2D::resize(float width, float height)
{
    m_width = width > 0.0f ? width : 1.0f;
    m_height = height > 0.0f ? height : 1.0f;
    rebuild();
}

const math::Mat4& Camera2D::projection() const noexcept
{
    return m_projection;
}

float Camera2D::width() const noexcept
{
    return m_width;
}

float Camera2D::height() const noexcept
{
    return m_height;
}

void Camera2D::rebuild()
{
    m_projection = math::Mat4::orthographic(0.0f, m_width, 0.0f, m_height, -1.0f, 1.0f);
}

}
