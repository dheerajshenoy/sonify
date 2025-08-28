#include "DTexture.hpp"

#include "raylib.h"

#include <cmath>

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
DTexture::resize(const std::array<int, 2> &dim, bool keepAspectRatio) noexcept
{
    if (dim == std::array{ -1, -1 }) { m_texture = LoadTexture(m_file_path); }
    else
    {
        Image image = LoadImage(m_file_path);
        if (!keepAspectRatio) { ImageResize(&image, dim[0], dim[1]); }
        else
        {
            // keep aspect ratio
            float scale = std::fminf((float)dim[0] / image.width,
                                     (float)dim[1] / image.height);

            int newW = (int)(image.width * scale);
            int newH = (int)(image.height * scale);
            ImageResize(&image, newW, newH);
        }
        m_texture = LoadTextureFromImage(image);
        UnloadImage(image);
    }
}
