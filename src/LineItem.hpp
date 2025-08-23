#pragma once

#include "DVector2.hpp"

#include <raylib.h>

class LineItem
{
private:

    Color m_color{ RED };
    DVector2<int> m_pos;
    int m_width, m_height;

public:

    inline void setPos(DVector2<int> pos) noexcept { m_pos = pos; }
    inline void setWidth(int w) noexcept { m_width = w; }
    inline void setHeight(int h) noexcept { m_height = h; }
    void render() noexcept;
    inline auto pos() const -> decltype(m_pos) { return m_pos; }
};
