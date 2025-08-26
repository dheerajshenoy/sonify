#include "DTexture.hpp"

#include "raylib.h"

DTexture::~DTexture()
{
    UnloadTexture(m_texture);
}

bool
DTexture::load(const char *filename) noexcept
{
    m_texture = LoadTexture(filename);
    return IsTextureValid(m_texture);
}

void
DTexture::render() noexcept
{
    DrawTexture(m_texture, m_pos.x, m_pos.y, WHITE);
}
