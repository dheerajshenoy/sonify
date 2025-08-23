#pragma once

#include "DVector2.hpp"
#include "sonify/Pixel.hpp"

#include <functional>
#include <print>
#include <raylib.h>

class PathItem
{
private:

    Color m_color{ RED };
    DVector2<int> m_pos;
    int m_width, m_height;
    std::vector<Pixel> m_pixels;

public:

    inline void setPointerPos(const DVector2<int> &pos) noexcept
    {
        m_pos = pos;
    }

    inline void appendPixel(const Pixel &pixel) noexcept
    {
        m_pixels.emplace_back(pixel);
    }

    inline std::vector<Pixel> &pixels() noexcept { return m_pixels; }
    inline void setPos(DVector2<int> pos) noexcept { m_pos = pos; }
    inline void setWidth(int w) noexcept { m_width = w; }
    inline void setHeight(int h) noexcept { m_height = h; }
    inline auto pointerPos() const -> decltype(m_pos) { return m_pos; }
    void render() noexcept;
};
