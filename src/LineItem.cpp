#include "LineItem.hpp"

#include <print>

void
LineItem::setPolarMode(bool state) noexcept
{
    m_polarMode = state;
    if (state)
    {
        m_renderFunction = [this]()
        {
            Vector2 center = { (float)m_pos.x, (float)m_pos.y };
            Vector2 size   = { (float)m_width, (float)m_height };
            DrawRectanglePro({ center.x - size.x / 2, center.y - size.y / 2,
                               size.x, size.y },
                             { size.x, size.y }, m_angle, m_color);
        };
    }
    else
    {
        m_renderFunction = [this]()
        { DrawRectangle(m_pos.x, m_pos.y, m_width, m_height, m_color); };
    }
}
