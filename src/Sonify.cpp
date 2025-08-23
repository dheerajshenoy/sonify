#include "Sonify.hpp"

#include "utils.hpp"

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

    OpenImage("/home/neo/Downloads/300x300-061-e1340955308953.jpg");
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

        BeginDrawing();
        {
            ClearBackground(WHITE);
            BeginMode2D(m_camera);
            {
                render();
            }
            EndMode2D();
        }
        EndDrawing();
    }
}

void
Sonify::OpenImage(const std::string &fileName) noexcept
{
    m_texture->load(fileName.c_str());
    m_texture->setPos({ 10, 10 });
    m_li->setPos(m_texture->pos());
    m_li->setWidth(1);
    m_li->setHeight(m_texture->height());
}

void
Sonify::render() noexcept
{
    m_texture->render();
    m_li->render();
}

void
Sonify::sonification() noexcept
{
    auto image    = LoadImageFromTexture(m_texture->texture());
    Color *pixels = LoadImageColors(image);
    int h         = image.height;
    int w         = image.width;

    std::vector<std::vector<short>> soundBuffer;
    std::vector<Pixel> pixelCol;

    for (int x = 0; x < w; x++)
    {
        pixelCol.clear();
        pixelCol.reserve(h);
        for (int y = 0; y < h; y++)
        {
            Color c = pixels[y * w + x];
            Pixel p = { RGBA{ c.r, c.g, c.b, c.a }, x, y };
            pixelCol.push_back(p);
        }

        soundBuffer.push_back(mapFunc(pixelCol));
    }

    for (auto &col : soundBuffer)
        m_audioBuffer.insert(m_audioBuffer.end(), col.begin(), col.end());

    UnloadImageColors(pixels);
    UnloadImage(image);
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
        f += mapper(0, 100, 0, 20000, hsv.h);
    }
    utils::generateSineWave(fs, 0.25, f, 0.005, 44100);
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
        else
        {

            out[i] = audio[gInstance->m_audioReadPos++];
            gInstance->setAudioReadPos(gInstance->m_audioReadPos /
                                       static_cast<float>(audio.size()) *
                                       gInstance->m_texture->texture().width);
        }
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
Sonify::setAudioReadPos(int pos) noexcept
{
    gInstance->m_li->setPos({ pos + 10, 10 });
}
