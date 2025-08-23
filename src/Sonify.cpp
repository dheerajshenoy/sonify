#include "Sonify.hpp"

#include "utils.hpp"

#include <cmath>
#include <cstring>
#include <functional>
#include <print>
#include <raylib.h>

Sonify::Sonify() noexcept
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "Sonify");
    SetTargetFPS(60);

    gInstance = this;

    OpenImage("/home/neo/Gits/sonifycpp/images/clock.png");
    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(4096);
    m_stream = LoadAudioStream(44100, 16, 1);
    SetAudioStreamCallback(
        m_stream, &Sonify::audioCallback); // let callback know which object

    m_camera          = { 0 };
    m_camera.target   = { 0, 0 };
    m_camera.offset   = { 0, 0 };
    m_camera.rotation = 0.0f;
    m_camera.zoom     = 1.0f;
    sonification();
    loop();
}

Sonify::~Sonify() noexcept
{
    CloseAudioDevice();
    StopAudioStream(m_stream);
    UnloadAudioStream(m_stream);
}

void
Sonify::loop() noexcept
{
    while (!WindowShouldClose())
    {
        handleMouseScroll();
        handleKeyEvents();
        m_cursorUpdater(m_audioReadPos);

        BeginDrawing();
        {
            ClearBackground(WHITE);
            BeginMode2D(m_camera);

            render();
            EndMode2D();
        }
        EndDrawing();
    }
}

void
Sonify::OpenImage(const std::string &fileName) noexcept
{
    m_texture->load(fileName.c_str());
    int x = GetScreenWidth() / 2 - m_texture->width() / 2;
    int y = GetScreenHeight() / 2 - m_texture->height() / 2;
    m_texture->setPos({ x, y });
}

void
Sonify::render() noexcept
{
    m_texture->render();
    if (m_li) m_li->render();
    if (m_ci) m_ci->render();
}

std::vector<short>
Sonify::mapFunc(const std::vector<Pixel> &pixelColumn) noexcept
{
    using freqMapFunc =
        std::function<double(double, double, double, double, double)>;
    freqMapFunc mapper = utils::LogMap;

    std::vector<short> fs;
    double f = 0;

    for (const auto &px : pixelColumn)
    {
        HSV hsv = utils::RGBtoHSV(px.rgba);
        f += mapper(0, 200, 0, 4000, hsv.v);
        // f += hsv.v;
        // if (intensity != 0) LOG("{}", intensity);
    }
    utils::generateSineWave(fs, 0.25, f, 0.01, 44100);
    return fs;
}

void
Sonify::audioCallback(void *buffer, unsigned int frames)
{
    if (!gInstance) return;

    int16_t *out = reinterpret_cast<int16_t *>(buffer);
    auto &audio  = gInstance->m_audioBuffer; // single vector

    for (unsigned int i = 0; i < frames; ++i)
    {
        // Loop when reaching the end
        if (gInstance->m_audioReadPos >= audio.size())
        {
            out[i]                        = 0;
            gInstance->m_finishedPlayback = true;
            gInstance->m_audioPlaying     = false;
        }
        else { out[i] = audio[(unsigned long)gInstance->m_audioReadPos++]; }
    }
}

void
Sonify::handleMouseScroll() noexcept
{
    float scroll     = GetMouseWheelMove();
    float zoomFactor = 1.1f;
    if (scroll != 0)
    {
        Vector2 mouse       = GetMousePosition();
        Vector2 worldBefore = GetScreenToWorld2D(mouse, m_camera);

        m_camera.zoom *= (scroll > 0) ? zoomFactor : 1.0f / zoomFactor;

        Vector2 worldAfter = GetScreenToWorld2D(mouse, m_camera);
        m_camera.target.x += worldBefore.x - worldAfter.x;
        m_camera.target.y += worldBefore.y - worldAfter.y;
    }
}

void
Sonify::handleKeyEvents() noexcept
{
    float panSpeed = 300.0f * GetFrameTime();

    if (IsKeyDown(KEY_W)) m_camera.target.y -= panSpeed;
    if (IsKeyDown(KEY_S)) m_camera.target.y += panSpeed;
    if (IsKeyDown(KEY_A)) m_camera.target.x -= panSpeed;
    if (IsKeyDown(KEY_D)) m_camera.target.x += panSpeed;
    if (IsKeyPressed(KEY_SPACE)) toggleAudioPlayback();
}

void
Sonify::toggleAudioPlayback() noexcept
{

    if (m_audioPlaying)
    {
        PauseAudioStream(m_stream);
        m_audioPlaying = false;
    }
    else
    {
        if (m_audioReadPos >= m_audioBuffer.size()) { m_audioReadPos = 0; }

        PlayAudioStream(m_stream);
        m_audioPlaying = true;
    }
}

void
Sonify::sonification() noexcept
{
    auto image    = LoadImageFromTexture(m_texture->texture());
    Color *pixels = LoadImageColors(image);
    int h         = image.height;
    int w         = image.width;

    std::vector<std::vector<short>> soundBuffer;

    updateCursorUpdater();

    switch (m_traversal_type)
    {
        case TraversalType::LEFT_TO_RIGHT:
            collectLeftToRight(pixels, w, h, soundBuffer);
            break;

        case TraversalType::RIGHT_TO_LEFT:
            collectRightToLeft(pixels, w, h, soundBuffer);
            break;

        case TraversalType::TOP_TO_BOTTOM:
            collectTopToBottom(pixels, w, h, soundBuffer);
            break;

        case TraversalType::BOTTOM_TO_TOP:
            collectBottomToTop(pixels, w, h, soundBuffer);
            break;

        case TraversalType::CIRCLE_INWARDS:
            collectCircleInwards(pixels, w, h, soundBuffer);
            break;

        case TraversalType::CIRCLE_OUTWARDS:
            collectCircleOutwards(pixels, w, h, soundBuffer);
            break;

        case TraversalType::CLOCKWISE:
            collectClockwise(pixels, w, h, soundBuffer);
            break;

        case TraversalType::ANTICLOCKWISE:
            collectAntiClockwise(pixels, w, h, soundBuffer);
            break;

        case TraversalType::PATH: collectPath(pixels, w, h, soundBuffer); break;

        case TraversalType::REGION:
            collectRegion(pixels, w, h, soundBuffer);
            break;
    }

    for (auto &col : soundBuffer)
        m_audioBuffer.insert(m_audioBuffer.end(), col.begin(), col.end());

    UnloadImageColors(pixels);
    UnloadImage(image);
}

void
Sonify::collectLeftToRight(Color *pixels, int w, int h,
                           std::vector<std::vector<short>> &buffer) noexcept
{

    std::vector<Pixel> pixelCol;
    pixelCol.reserve(h);

    for (int x = 0; x < w; x++)
    {
        pixelCol.clear();
        for (int y = 0; y < h; y++)
        {
            const auto &px = pixels[y * w + x];
            pixelCol.push_back({ RGBA{ px.r, px.g, px.b, px.a }, x, y });
        }
        buffer.push_back(mapFunc(pixelCol));
    }
}

void
Sonify::collectRightToLeft(Color *pixels, int w, int h,
                           std::vector<std::vector<short>> &buffer) noexcept
{

    std::vector<Pixel> pixelCol;
    pixelCol.reserve(h);

    for (int x = w; x > 0; x--)
    {
        pixelCol.clear();
        for (int y = 0; y < h; y++)
        {
            const auto &px = pixels[y * w + x];
            pixelCol.push_back({ RGBA{ px.r, px.g, px.b, px.a }, x, y });
        }
        buffer.push_back(mapFunc(pixelCol));
    }
}

void
Sonify::collectTopToBottom(Color *pixels, int w, int h,
                           std::vector<std::vector<short>> &buffer) noexcept
{
    std::vector<Pixel> pixelCol;
    pixelCol.reserve(h);

    for (int y = 0; y < h; y++)
    {
        pixelCol.clear();
        for (int x = 0; x < w; x++)
        {
            const auto &px = pixels[y * w + x];
            pixelCol.push_back({ RGBA{ px.r, px.g, px.b, px.a }, x, y });
        }
        buffer.push_back(mapFunc(pixelCol));
    }
}

void
Sonify::collectBottomToTop(Color *pixels, int w, int h,
                           std::vector<std::vector<short>> &buffer) noexcept
{
    std::vector<Pixel> pixelCol;
    pixelCol.reserve(h);

    for (int y = h; y >= 0; y--)
    {
        pixelCol.clear();
        for (int x = 0; x < w; x++)
        {
            const auto &px = pixels[y * w + x];
            pixelCol.push_back({ RGBA{ px.r, px.g, px.b, px.a }, x, y });
        }
        buffer.push_back(mapFunc(pixelCol));
    }
}

void
Sonify::collectCircleOutwards(Color *pixels, int w, int h,
                              std::vector<std::vector<short>> &buffer) noexcept
{
    int cx = w / 2;
    int cy = h / 2;

    int maxRadius = std::max(cx, w - cx - 1);
    maxRadius     = std::max(maxRadius, std::max(cy, h - cy - 1));

    std::vector<Pixel> pixelCol;

    for (int r = 0; r <= maxRadius; ++r)
    {
        pixelCol.clear();

        // Loop over bounding box of the current radius
        int left   = std::max(0, cx - r);
        int right  = std::min(w - 1, cx + r);
        int top    = std::max(0, cy - r);
        int bottom = std::min(h - 1, cy + r);

        // Top row: left → right
        for (int x = left; x <= right; ++x)
        {
            const auto &px = pixels[top * w + x];
            pixelCol.push_back({ RGBA{ px.r, px.g, px.b, px.a }, x, top });
        }

        // Right column: top+1 → bottom
        for (int y = top + 1; y <= bottom; ++y)
        {
            const auto &px = pixels[y * w + right];
            pixelCol.push_back({ RGBA{ px.r, px.g, px.b, px.a }, right, y });
        }

        // Bottom row: right-1 → left (if top != bottom)
        if (top != bottom)
        {
            for (int x = right - 1; x >= left; --x)
            {
                const auto &px = pixels[bottom * w + x];
                pixelCol.push_back(
                    { RGBA{ px.r, px.g, px.b, px.a }, x, bottom });
            }
        }

        // Left column: bottom-1 → top+1 (if left != right)
        if (left != right)
        {
            for (int y = bottom - 1; y > top; --y)
            {
                const auto &px = pixels[y * w + left];
                pixelCol.push_back({ RGBA{ px.r, px.g, px.b, px.a }, left, y });
            }
        }

        if (!pixelCol.empty()) buffer.push_back(mapFunc(pixelCol));
    }
}

void
Sonify::collectCircleInwards(Color *pixels, int w, int h,
                             std::vector<std::vector<short>> &buffer) noexcept
{
    int left   = 0;
    int right  = w - 1;
    int top    = 0;
    int bottom = h - 1;

    std::vector<Pixel> pixelCol;

    while (left <= right && top <= bottom)
    {
        pixelCol.clear();

        // Top row: left → right
        for (int x = left; x <= right; ++x)
        {
            const auto &px = pixels[top * w + x];
            pixelCol.push_back({ RGBA{ px.r, px.g, px.b, px.a }, x, top });
        }

        // Right column: top+1 → bottom
        for (int y = top + 1; y <= bottom; ++y)
        {
            const auto &px = pixels[y * w + right];
            pixelCol.push_back({ RGBA{ px.r, px.g, px.b, px.a }, right, y });
        }

        // Bottom row: right-1 → left (if top != bottom)
        if (top != bottom)
        {
            for (int x = right - 1; x >= left; --x)
            {
                const auto &px = pixels[bottom * w + x];
                pixelCol.push_back(
                    { RGBA{ px.r, px.g, px.b, px.a }, x, bottom });
            }
        }

        // Left column: bottom-1 → top+1 (if left != right)
        if (left != right)
        {
            for (int y = bottom - 1; y > top; --y)
            {
                const auto &px = pixels[y * w + left];
                pixelCol.push_back({ RGBA{ px.r, px.g, px.b, px.a }, left, y });
            }
        }

        buffer.push_back(mapFunc(pixelCol));

        // Move to inner circle
        left++;
        right--;
        top++;
        bottom--;
    }
}

void
Sonify::collectPath(Color *pixels, int w, int h,
                    std::vector<std::vector<short>> &buffer) noexcept
{
}

void
Sonify::collectRegion(Color *pixels, int w, int h,
                      std::vector<std::vector<short>> &buffer) noexcept
{
}

void
Sonify::collectAntiClockwise(Color *pixels, int w, int h,
                             std::vector<std::vector<short>> &buffer) noexcept
{
    int cx     = w / 2;
    int cy     = h / 2;
    int length = static_cast<int>(std::sqrt(cx * cx + cy * cy));

    for (int angle = 0; angle < 360; angle++)
    {
        float rad  = -angle * (M_PI / 180.0f);
        float cosA = std::cos(rad);
        float sinA = std::sin(rad);

        std::vector<Pixel> pixelCol;
        pixelCol.reserve(length);

        for (int r = 0; r < length; r++)
        {
            int x = cx + static_cast<int>(r * cosA);
            int y = cy + static_cast<int>(r * sinA);

            if (x >= 0 && x < w && y >= 0 && y < h)
            {
                const auto &p = pixels[y * w + x];
                pixelCol.emplace_back(
                    Pixel{ RGBA{ p.r, p.g, p.b, p.a }, x, y });
            }
            else { break; }
        }

        buffer.push_back(mapFunc(pixelCol));
    }
}

void
Sonify::collectClockwise(Color *pixels, int w, int h,
                         std::vector<std::vector<short>> &buffer) noexcept
{
    int cx     = w / 2;
    int cy     = h / 2;
    int length = static_cast<int>(std::sqrt(cx * cx + cy * cy));

    for (int angle = 0; angle < 360; angle++)
    {
        float rad  = angle * (M_PI / 180.0f);
        float cosA = std::cos(rad);
        float sinA = std::sin(rad);

        std::vector<Pixel> pixelCol;
        pixelCol.reserve(length);

        for (int r = 0; r < length; r++)
        {
            int x = cx + static_cast<int>(r * cosA);
            int y = cy + static_cast<int>(r * sinA);

            if (x >= 0 && x < w && y >= 0 && y < h)
            {
                const auto &p = pixels[y * w + x];
                pixelCol.emplace_back(
                    Pixel{ RGBA{ p.r, p.g, p.b, p.a }, x, y });
            }
            else { break; }
        }

        buffer.push_back(mapFunc(pixelCol));
    }
}

void
Sonify::updateCursorUpdater() noexcept
{
    const int imgw              = m_texture->width();
    const int imgh              = m_texture->height();
    const DVector2<int> &imgpos = m_texture->pos();

    switch (m_traversal_type)
    {
        case TraversalType::LEFT_TO_RIGHT:
            if (!m_li) m_li = new LineItem();
            m_li->setHeight(imgh);
            m_li->setPolarMode(false);
            m_li->setWidth(1);
            m_cursorUpdater = [this, imgw, imgpos](int audioPos)
            {
                float progress =
                    (float)audioPos / static_cast<float>(m_audioBuffer.size());
                m_li->setPos({ progress * imgw + imgpos.x, imgpos.y });
            };
            break;

        case TraversalType::RIGHT_TO_LEFT:
            if (!m_li) m_li = new LineItem();
            m_li->setHeight(imgh);
            m_li->setPolarMode(false);
            m_li->setWidth(1);
            m_cursorUpdater = [this, imgpos, imgw](int audioPos)
            {
                const float progress =
                    (float)audioPos / static_cast<float>(m_audioBuffer.size());
                m_li->setPos({ imgpos.x + imgw - progress * imgw, imgpos.y });
            };
            break;

        case TraversalType::TOP_TO_BOTTOM:
            if (!m_li) m_li = new LineItem();
            m_li->setPolarMode(false);
            m_li->setWidth(imgw);
            m_li->setHeight(1);
            m_cursorUpdater = [this, imgpos, imgh](int audioPos)
            {
                const float progress =
                    (float)audioPos / static_cast<float>(m_audioBuffer.size());
                m_li->setPos({ imgpos.x, imgpos.y + progress * imgh });
            };
            break;

        case TraversalType::BOTTOM_TO_TOP:
            if (!m_li) m_li = new LineItem();
            m_li->setPolarMode(false);
            m_li->setWidth(imgw);
            m_li->setHeight(1);
            m_cursorUpdater = [this, imgh, imgpos](int audioPos)
            {
                const float progress =
                    (float)audioPos / static_cast<float>(m_audioBuffer.size());
                m_li->setPos({ imgpos.x, imgpos.y + imgh - progress * imgh });
            };
            break;
        case TraversalType::CIRCLE_INWARDS:
        {
            if (!m_ci) m_ci = new CircleItem();
            const float maxr =
                std::sqrt(imgw * imgw / 4.0f + imgh * imgh / 4.0f);

            m_ci->setCenter({ (float)imgpos.x + imgw / 2.0f,
                              (float)imgpos.y + imgh / 2.0f });
            m_cursorUpdater = [this, imgw, imgh, maxr](int audioPos)
            {
                const float progress =
                    (float)audioPos / static_cast<float>(m_audioBuffer.size());
                m_ci->setRadius(maxr - progress * maxr);
            };
        }
        break;
        case TraversalType::CIRCLE_OUTWARDS:
        {
            if (!m_ci) m_ci = new CircleItem();

            const float maxr =
                std::sqrt(imgw * imgw / 4.0f + imgh * imgh / 4.0f);

            m_ci->setCenter({ (float)m_texture->pos().x + imgw / 2.0f,
                              (float)m_texture->pos().y + imgh / 2.0f });
            m_cursorUpdater = [this, imgw, imgh, maxr](int audioPos)
            {
                const float progress =
                    (float)audioPos / static_cast<float>(m_audioBuffer.size());
                m_ci->setRadius(progress * maxr);
            };
        }
        break;

        case TraversalType::CLOCKWISE:
        {
            if (!m_li) m_li = new LineItem();
            m_li->setPolarMode(true);
            m_li->setHeight(1);
            m_li->setWidth(imgw);
            m_li->setPos({ imgpos.x + imgw, imgpos.y + imgh / 2 });
            m_cursorUpdater = [this](int audioPos)
            {
                const float progress =
                    (float)audioPos / static_cast<float>(m_audioBuffer.size());
                m_li->setAngle(progress * 360.0f + 180.0f);
            };
        }
        break;

        case TraversalType::ANTICLOCKWISE:
        {
            if (!m_li) m_li = new LineItem();
            m_li->setPolarMode(true);
            m_li->setHeight(1);
            m_li->setWidth(imgw);
            m_li->setPos({ imgpos.x + imgw, imgpos.y + imgh / 2 });
            m_cursorUpdater = [this](int audioPos)
            {
                const float progress =
                    (float)audioPos / static_cast<float>(m_audioBuffer.size());
                m_li->setAngle(-progress * 360.0f + 180.0f);
            };
        }
        break;
        case TraversalType::PATH:
        case TraversalType::REGION: break;
    }
}
