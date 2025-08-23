#include "DTexture.hpp"

DTexture::~DTexture()
{
    UnloadTexture(m_texture);
}

void
DTexture::load(const char *filename) noexcept
{
    m_texture = LoadTexture(filename);
}

void
DTexture::render() noexcept
{
    DrawTexture(m_texture, m_pos.x, m_pos.y, WHITE);
}
