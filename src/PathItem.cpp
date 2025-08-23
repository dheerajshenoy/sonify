#include "PathItem.hpp"

#include <raylib.h>

void
PathItem::render() noexcept
{
    if (m_pixels.size() < 2) return;

    DrawCircle(m_pos.x, m_pos.y, 10, GREEN);

    for (size_t i = 1; i < m_pixels.size(); ++i)
    {
        DrawLine(m_pixels[i - 1].x, m_pixels[i - 1].y, m_pixels[i].x,
                 m_pixels[i].y, RED);
    }
}
