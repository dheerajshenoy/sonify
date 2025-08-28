#pragma once

#include "DVector2.hpp"
#include "raylib.h"

#include <array>
#include <string>

class DTexture
{
private:

    const char *m_file_path;
    Texture2D m_texture;
    DVector2<int> m_pos;

public:

    ~DTexture();
    inline int width() const noexcept { return m_texture.width; }
    inline int height() const noexcept { return m_texture.height; }
    void render() noexcept;
    bool load(const char *filename) noexcept;
    Texture2D texture() const noexcept { return m_texture; }
    inline void setPos(const DVector2<int> &pos) noexcept { m_pos = pos; }
    void resize(const std::array<int, 2> &dim = { -1, -1 }) noexcept;
    auto pos() const noexcept -> decltype(m_pos) { return m_pos; }
};
