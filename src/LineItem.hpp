#pragma once

#include "DVector2.hpp"

#include <functional>
#include <raylib.h>

class LineItem
{
private:

    Color m_color{ RED };
    DVector2<int> m_pos;
    int m_width, m_height;
    bool m_polarMode{ false };
    float m_angle{ 0.0f };

    using RenderFunction = std::function<void()>;
    RenderFunction m_renderFunction;

public:

    void setPolarMode(bool state) noexcept;

    inline void setAngle(float angle) noexcept { m_angle = angle; }
    inline bool polarMode() const noexcept { return m_polarMode; }
    inline void setPos(DVector2<int> pos) noexcept { m_pos = pos; }
    inline void setWidth(int w) noexcept { m_width = w; }
    inline void setHeight(int h) noexcept { m_height = h; }
    inline auto pos() const -> decltype(m_pos) { return m_pos; }
    inline void render() const
    {
        if (m_renderFunction) m_renderFunction();
    }
};
