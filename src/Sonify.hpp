#pragma once

#include "CircleItem.hpp"
#include "Config.hpp"
#include "DTexture.hpp"
#include "LineItem.hpp"
#include "PathItem.hpp"
#include "PixelMapManager.hpp"
#include "Timer.hpp"
#include "argparse.hpp"
#include "raylib.h"
#include "sonify/DefaultPixelMappings/HSVMap.hpp"
#include "sonify/DefaultPixelMappings/IntensityMap.hpp"
#include "sonify/Pixel.hpp"

#include <fftw3.h>
#include <functional>
#include <print>
#include <string>

#define LOG(...)         std::println(__VA_ARGS__);
#define __SONIFY_VERSION "0.1.0"

class Sonify
{
public:

    Sonify(const argparse::ArgumentParser &) noexcept;
    ~Sonify() noexcept;

    bool OpenImage(std::string fileName) noexcept;

private:

    static void audioCallback(void *bufferData, unsigned int frames);

    void sonification() noexcept;
    void loop() noexcept;
    void render() noexcept;
    void handleMouseScroll() noexcept;
    void handleMouseEvents() noexcept;
    void handleKeyEvents() noexcept;
    void toggleAudioPlayback() noexcept;
    void updateCursorUpdater() noexcept;
    std::string replaceHome(const std::string_view &str) noexcept;

    using AudioBuffer = std::vector<std::vector<short>>;
    void collectLeftToRight(Color *pixels, int w, int h,
                            AudioBuffer &buffer) noexcept;

    void collectRightToLeft(Color *pixels, int w, int h,
                            AudioBuffer &buffer) noexcept;

    void collectTopToBottom(Color *pixels, int w, int h,
                            AudioBuffer &buffer) noexcept;

    void collectBottomToTop(Color *pixels, int w, int h,
                            AudioBuffer &buffer) noexcept;

    void collectClockwise(Color *pixels, int w, int h,
                          AudioBuffer &buffer) noexcept;

    void collectCircleOutwards(Color *pixels, int w, int h,
                               AudioBuffer &buffer) noexcept;

    void collectCircleInwards(Color *pixels, int w, int h,
                              AudioBuffer &buffer) noexcept;

    void collectRegion(Color *pixels, int w, int h,
                       AudioBuffer &buffer) noexcept;

    void collectAntiClockwise(Color *pixels, int w, int h,
                              AudioBuffer &buffer) noexcept;
    void parse_args(const argparse::ArgumentParser &) noexcept;
    void setSamplerate(int SR) noexcept;
    void recenterView() noexcept;
    void centerImage() noexcept;
    void seekCursor(float seconds) noexcept;
    void saveAudio(const std::string &fileName) noexcept;
    void loadUserPixelMappings() noexcept;
    void loadPixelMappingsSharedObjects(const std::string &dir) noexcept;
    void loadDefaultPixelMappings() noexcept;
    constexpr Color ColorFromHex(unsigned int hex) noexcept
    {
        Color c;
        if (hex <= 0xFFFFFF) // RRGGBB
        {
            c.r = (hex >> 16) & 0xFF;
            c.g = (hex >> 8) & 0xFF;
            c.b = (hex) & 0xFF;
            c.a = 255;
        }
        else // RRGGBBAA
        {
            c.r = (hex >> 24) & 0xFF;
            c.g = (hex >> 16) & 0xFF;
            c.b = (hex >> 8) & 0xFF;
            c.a = (hex) & 0xFF;
        }
        return c;
    }
    void showDragDropText() noexcept;
    void handleFileDrop() noexcept;

private:

    using PixelMapFunc =
        std::function<std::vector<short>(const std::vector<Pixel> &)>;
    PixelMapFunc m_mapFunc;
    using CursorUpdater = std::function<void(int pos)>;
    CursorUpdater m_cursorUpdater;
    enum class TraversalType
    {
        LEFT_TO_RIGHT = 0,
        RIGHT_TO_LEFT,
        TOP_TO_BOTTOM,
        BOTTOM_TO_TOP,
        CIRCLE_INWARDS,
        CIRCLE_OUTWARDS,
        CLOCKWISE,
        ANTICLOCKWISE,
        PATH,
        REGION
    };
    DTexture *m_texture{ new DTexture() };
    Image m_image;
    AudioStream m_stream{ 0 };
    std::vector<short> m_audioBuffer;
    std::string m_saveFileName;
    std::string m_pixelMapName{ "Intensity" };
    LineItem *m_li{ nullptr };
    CircleItem *m_ci{ nullptr };
    PathItem *m_pi{ nullptr };

    bool m_finishedPlayback{ true };
    bool m_audioPlaying{ false };
    bool m_isSonified{ false };
    bool m_showNotSonifiedMessage{ false };
    bool m_showDragDropText{ true };

    float m_showNotSonifiedMessageTimer{ 1.5f };
    int m_audioReadPos{ 0 };
    int m_sampleRate{ 44100 };
    int m_channels{ 1 };
    int m_fps{ 60 };
    TraversalType m_traversal_type{ 0 };
    Camera2D m_camera;
    int m_screenW, m_screenH;
    PixelMapManager *m_pixelMapManager{ nullptr };
    Color m_bg{ ColorFromHex(0x000000) };
    std::string m_dragDropText{ "Drop an image file here to sonify" };
    Color m_dragDropTextColor{ DARKGRAY };

    Config m_config;
    Font m_font;
    Timer m_timer;
};

static Sonify *gInstance{ nullptr };
