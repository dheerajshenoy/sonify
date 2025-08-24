#include "Sonify.hpp"

#include "PixelMapManager.hpp"
#include "raylib.h"
#include "sonify/utils.hpp"

#include <cmath>
#include <cstring>
#include <dlfcn.h>
#include <filesystem>
#include <functional>
#include <type_traits>

Sonify::Sonify(const argparse::ArgumentParser &args) noexcept
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(0, 0, "Sonify");
    m_screenW = GetScreenWidth();
    m_screenH = GetScreenHeight();
    SetWindowMinSize(800, 600);
    SetTargetFPS(m_fps);

    gInstance = this;

    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(4096);
    setSamplerate(m_sampleRate);

    m_camera          = { 0 };
    m_camera.target   = { 0, 0 };
    m_camera.offset   = { 0, 0 };
    m_camera.rotation = 0.0f;
    m_camera.zoom     = 1.0f;

    loadUserPixelMappings();
    parse_args(args);
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
        int newW = GetScreenWidth();
        int newH = GetScreenHeight();
        if (newW != m_screenW || newH != m_screenH)
        {
            m_screenW = newW;
            m_screenH = newH;
        }
        //
        //     if (m_texture)
        //     {
        //         int x = m_screenW / 2 - m_texture->width() / 2;
        //         int y = m_screenH / 2 - m_texture->height() / 2;
        //         m_texture->setPos({ x, y });
        //     }
        // }
        if (m_traversal_type == TraversalType::PATH) handleMouseEvents();
        handleMouseScroll();
        handleKeyEvents();
        if (m_audioPlaying && m_cursorUpdater) m_cursorUpdater(m_audioReadPos);

        BeginDrawing();
        {

            ClearBackground(m_bg);

            BeginMode2D(m_camera);

            render();
            EndMode2D();
            if (m_showNotSonifiedMessage)
            {
                m_showNotSonifiedMessageTimer -= GetFrameTime();
                if (m_showNotSonifiedMessageTimer <= 0.0f)
                {
                    m_showNotSonifiedMessage      = false;
                    m_showNotSonifiedMessageTimer = 1.5f;
                }

                DrawText("Press `J` to sonify first", 10, 10, 20, RED);
            }
        }
        EndDrawing();
    }
}

void
Sonify::OpenImage(std::string fileName) noexcept
{
    if (!fileName.empty()) fileName = replaceHome(fileName);

    m_texture->load(fileName.c_str());
    const int x = m_screenW / 2 - m_texture->width() / 2;
    const int y = m_screenH / 2 - m_texture->height() / 2;
    m_texture->setPos({ x, y });
    m_image = LoadImageFromTexture(m_texture->texture());
}

void
Sonify::render() noexcept
{

    m_texture->render();
    if (m_li) m_li->render();
    if (m_ci) m_ci->render();
    if (m_pi) m_pi->render();
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
    }
    utils::generateSineWave(fs, 0.5, f, 0.01, m_sampleRate);
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
    if (IsKeyPressed(KEY_ZERO)) recenterView();
    if (IsKeyPressed(KEY_J)) sonification();
    if (IsKeyPressed(KEY_COMMA)) seekCursor(-1);
    if (IsKeyPressed(KEY_PERIOD)) seekCursor(1);
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
        if (!m_isSonified)
        {
            m_showNotSonifiedMessage = true;
            return;
        }
        if (m_audioReadPos >= m_audioBuffer.size()) { m_audioReadPos = 0; }

        PlayAudioStream(m_stream);
        m_audioPlaying = true;
    }
}

void
Sonify::sonification() noexcept
{
    Color *pixels = LoadImageColors(m_image);
    int h         = m_image.height;
    int w         = m_image.width;

    AudioBuffer soundBuffer;
    m_audioBuffer.clear();

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

        case TraversalType::PATH:
        {
            const auto &pathPixels = m_pi->pixels();
            for (const auto &p : pathPixels)
            {
                std::vector<Pixel> pixelGroup(
                    10, p); // Repeat pixel 10 times for more audio
                soundBuffer.push_back(mapFunc(pixelGroup));
            }
        }
        break;

        case TraversalType::REGION:
            collectRegion(pixels, w, h, soundBuffer);
            break;
    }

    for (auto &col : soundBuffer)
        m_audioBuffer.insert(m_audioBuffer.end(), col.begin(), col.end());

    if (m_cursorUpdater) m_cursorUpdater(0);
    UnloadImageColors(pixels);
    m_isSonified = true;
}

void
Sonify::collectLeftToRight(Color *pixels, int w, int h,
                           AudioBuffer &buffer) noexcept
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
                           AudioBuffer &buffer) noexcept
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
                           AudioBuffer &buffer) noexcept
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
                           AudioBuffer &buffer) noexcept
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
                              AudioBuffer &buffer) noexcept
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
                             AudioBuffer &buffer) noexcept
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
Sonify::collectRegion(Color *pixels, int w, int h, AudioBuffer &buffer) noexcept
{
}

void
Sonify::collectAntiClockwise(Color *pixels, int w, int h,
                             AudioBuffer &buffer) noexcept
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
                         AudioBuffer &buffer) noexcept
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
        {
            if (!m_li) m_li = new LineItem();
            m_li->setHeight(imgh);
            m_li->setPolarMode(false);
            m_li->setWidth(10);
            m_cursorUpdater = [this, imgw, imgpos](int audioPos)
            {
                float progress =
                    (float)audioPos / static_cast<float>(m_audioBuffer.size());
                m_li->setPos({ progress * imgw + imgpos.x, imgpos.y });
            };
        }
        break;

        case TraversalType::RIGHT_TO_LEFT:
        {
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
        }
        break;

        case TraversalType::TOP_TO_BOTTOM:
        {
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
        }
        break;

        case TraversalType::BOTTOM_TO_TOP:
        {
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
        }
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
            m_li->setHeight(5);
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
        {
            if (!m_pi) m_pi = new PathItem();
            auto pixels = m_pi->pixels();

            m_cursorUpdater = [this, pixels](int audioPos)
            {
                if (pixels.empty()) return;

                const float progress =
                    (float)audioPos / static_cast<float>(m_audioBuffer.size());
                int pixelIndex = static_cast<int>(progress * pixels.size());
                pixelIndex =
                    std::min(pixelIndex, static_cast<int>(pixels.size() - 1));

                const Pixel &pixel = pixels.at(pixelIndex);
                m_pi->setPointerPos({ pixel.x, pixel.y });
            };
        }
        break;

        case TraversalType::REGION: break;
    }
}

void
Sonify::handleMouseEvents() noexcept
{

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        if (!m_pi) m_pi = new PathItem();

        Vector2 mouse      = GetMousePosition();
        Vector2 mouseWorld = GetScreenToWorld2D(mouse, m_camera);

        const DVector2<int> &imgPos = m_texture->pos();
        const int width             = m_texture->width();
        const int height            = m_texture->height();

        // check if inside image bounds
        if (mouseWorld.x >= imgPos.x && mouseWorld.x < imgPos.x + width &&
            mouseWorld.y >= imgPos.y && mouseWorld.y < imgPos.y + height)
        {
            // translate to image-local coords
            int px = (int)(mouseWorld.x - imgPos.x);
            int py = (int)(mouseWorld.y - imgPos.y);

            // NOTE: GetImageColor expects uncompressed image in RAM
            Color c = GetImageColor(m_image, px, py);

            m_pi->appendPixel({ RGBA{ c.r, c.g, c.b, c.a }, (int)mouseWorld.x,
                                (int)mouseWorld.y });
        }
    }
}

void
Sonify::parse_args(const argparse::ArgumentParser &args) noexcept
{
    if (args.is_used("--samplerate"))
        setSamplerate(args.get<int>("--samplerate"));

    // if (args.is_used("--channels"))
    //     setChannels(std::stoi(args.get("--channels")));

    if (args.is_used("--traversal"))
        m_traversal_type =
            static_cast<TraversalType>(args.get<int>("--traversal"));

    if (args.is_used("--background"))
        m_bg = ColorFromHex(args.get<unsigned int>("--background"));

    if (args.is_used("--fps")) m_fps = std::stoi(args.get("--fps"));
    if (args.is_used("FILE")) { OpenImage(args.get("FILE")); }
}

void
Sonify::setSamplerate(int SR) noexcept
{
    m_sampleRate = SR;
    if (IsAudioStreamValid(m_stream))
    {
        StopAudioStream(m_stream);
        UnloadAudioStream(m_stream);
    }
    m_stream = LoadAudioStream(m_sampleRate, 16, m_channels);
    SetAudioStreamCallback(m_stream, &Sonify::audioCallback);
}

void
Sonify::recenterView() noexcept
{
    m_camera.target   = { 0, 0 };
    m_camera.offset   = { 0, 0 };
    m_camera.rotation = 0.0f;
    m_camera.zoom     = 1.0f;
}

void
Sonify::seekCursor(float seconds) noexcept
{
    // samples per second (mono = sampleRate * 1, stereo = sampleRate * 2, etc.)
    size_t samplesPerSecond = m_sampleRate * m_channels;

    long long offset = static_cast<long long>(seconds * samplesPerSecond);
    long long newPos = static_cast<long long>(m_audioReadPos) + offset;

    if (newPos < 0) newPos = 0;
    if (newPos >= static_cast<long long>(m_audioBuffer.size()))
        newPos = m_audioBuffer.size() - 1;

    m_audioReadPos = static_cast<size_t>(newPos);
    m_cursorUpdater(m_audioReadPos);
}

void
Sonify::saveAudio(const std::string &fileName) noexcept
{
    if (!m_isSonified || m_audioBuffer.empty() || fileName.empty()) return;

    Wave wave = { .frameCount = static_cast<unsigned int>(m_audioBuffer.size() /
                                                          m_channels),
                  .sampleRate = m_sampleRate,
                  .sampleSize = 16,
                  .channels   = m_channels,
                  .data       = (void *)m_audioBuffer.data() };
    const std::string &path = replaceHome(fileName);
    if (!ExportWave(wave, fileName.c_str()))
    {
        std::cerr << "Unable to export wave";
    }
}

std::string
Sonify::replaceHome(const std::string_view &str) noexcept
{
    if (!str.empty() && str.front() == '~')
    {
        return std::string(std::getenv("HOME")) + std::string(str.substr(1));
    }
    return std::string(str);
}

// load all the user defined pixel mappings
void
Sonify::loadUserPixelMappings() noexcept
{
    m_pixelMapManager = new PixelMapManager();

    namespace fs = std::filesystem;
    const std::string &config_dir =
        std::getenv("HOME") + std::string("/.config/sonify");

    if (!fs::exists(config_dir))
        fs::create_directory(config_dir);
    else
    {
        // load mappings
        const std::string &mappings_dir = config_dir + "/mappings";
        if (fs::exists(mappings_dir))
            loadPixelMappingsSharedObjects(mappings_dir);
    }
}

void
Sonify::loadPixelMappingsSharedObjects(const std::string &dir) noexcept
{
    namespace fs = std::filesystem;
    for (const auto &entry : fs::directory_iterator(dir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".so")
        {
            void *handle = dlopen(entry.path().c_str(), RTLD_LAZY);

            if (handle)
            {
                MapTemplate *m =
                    reinterpret_cast<MapTemplate *>(dlsym(handle, "create"));
                if (!m)
                {
                    TraceLog(LOG_WARNING, dlerror());
                    dlclose(handle);
                    continue;
                }
                m_pixelMapManager->addMap({ handle, m });
            }
        }
    }
}
