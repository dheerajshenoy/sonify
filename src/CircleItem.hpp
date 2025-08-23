#pragma once

#include <raylib.h>

class CircleItem
{
private:

    Color m_color{ RED };
    Vector2 m_center;
    float m_radius{ 1.0f };

public:

    inline void setCenter(Vector2 center) noexcept { m_center = center; }
    inline void setRadius(float r) noexcept { m_radius = r; }
    inline float radius() const noexcept { return m_radius; }
    void render() noexcept;
    inline Vector2 center() const { return m_center; }
};
