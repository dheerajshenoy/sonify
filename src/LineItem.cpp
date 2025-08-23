#include "LineItem.hpp"

void
LineItem::render() noexcept
{
    DrawRectangle(m_pos.x, m_pos.y, m_width, m_height, m_color);
}
