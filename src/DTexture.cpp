#include "DTexture.hpp"

#include "raylib.h"

DTexture::~DTexture()
{
    UnloadTexture(m_texture);
}

bool
DTexture::load(const char *filename) noexcept
{
    m_file_path = filename;
    m_texture   = LoadTexture(filename);
    return IsTextureValid(m_texture);
}

void
DTexture::render() noexcept
{
    DrawTexture(m_texture, m_pos.x, m_pos.y, WHITE);
}

void
DTexture::resize(const std::array<int, 2> &dim) noexcept
{
    if (dim == std::array{ -1, -1 }) { m_texture = LoadTexture(m_file_path); }
    else
    {
        Image image = LoadImage(m_file_path);
        ImageResize(&image, dim[0], dim[1]);
        m_texture = LoadTextureFromImage(image);
    }
}
