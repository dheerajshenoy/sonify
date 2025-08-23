#include "CircleItem.hpp"

void
CircleItem::render() noexcept
{
    DrawCircleLinesV(m_center, m_radius, m_color);
}
