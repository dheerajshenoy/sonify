#pragma once

#include "CircleItem.hpp"
#include "DTexture.hpp"
#include "LineItem.hpp"
#include "PathItem.hpp"
#include "argparse.hpp"
#include "raylib.h"
#include "sonify/Pixel.hpp"

#include <fftw3.h>
#include <functional>
#include <string>

#define LOG(...)         std::println(__VA_ARGS__);
#define __SONIFY_VERSION "0.1.0"

class Sonify
{
public:

    Sonify(const argparse::ArgumentParser &) noexcept;
    ~Sonify() noexcept;

    void OpenImage(std::string fileName) noexcept;

private:

    static void audioCallback(void *bufferData, unsigned int frames);
    std::vector<short> mapFunc(const std::vector<Pixel> &) noexcept;
    void sonification() noexcept;
    void loop() noexcept;
    void render() noexcept;
    void handleMouseScroll() noexcept;
    void handleMouseEvents() noexcept;
    void handleKeyEvents() noexcept;
    void toggleAudioPlayback() noexcept;
    void updateCursorUpdater() noexcept;
    void collectLeftToRight(Color *pixels, int w, int h,
                            std::vector<std::vector<short>> &buffer) noexcept;

    void collectRightToLeft(Color *pixels, int w, int h,
                            std::vector<std::vector<short>> &buffer) noexcept;

    void collectTopToBottom(Color *pixels, int w, int h,
                            std::vector<std::vector<short>> &buffer) noexcept;

    void collectBottomToTop(Color *pixels, int w, int h,
                            std::vector<std::vector<short>> &buffer) noexcept;

    void collectClockwise(Color *pixels, int w, int h,
                          std::vector<std::vector<short>> &buffer) noexcept;
    void
    collectCircleOutwards(Color *pixels, int w, int h,
                          std::vector<std::vector<short>> &buffer) noexcept;

    void collectCircleInwards(Color *pixels, int w, int h,
                              std::vector<std::vector<short>> &buffer) noexcept;

    void collectRegion(Color *pixels, int w, int h,
                       std::vector<std::vector<short>> &buffer) noexcept;

    void collectAntiClockwise(Color *pixels, int w, int h,
                              std::vector<std::vector<short>> &buffer) noexcept;
    void parse_args(const argparse::ArgumentParser &) noexcept;

    using CursorUpdater = std::function<void(int pos)>;
    CursorUpdater m_cursorUpdater;

    void setSamplerate(int SR) noexcept;
    void recenterView() noexcept;

    void seekCursor(float seconds) noexcept;

private:

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
    AudioStream m_stream;
    std::vector<short> m_audioBuffer;
    LineItem *m_li{ nullptr };
    CircleItem *m_ci{ nullptr };
    PathItem *m_pi{ nullptr };
    bool m_finishedPlayback{ true }, m_audioPlaying{ false },
        m_isSonified{ false }, m_showNotSonifiedMessage{ false };
    float m_showNotSonifiedMessageTimer{ 1.5f };
    int m_audioReadPos{ 0 };
    int m_sampleRate{ 44100 };
    int m_channels{ 1 };
    int m_fps{ 60 };
    TraversalType m_traversal_type{ 0 };
    Camera2D m_camera;
    int m_screenW, m_screenH;
};

static Sonify *gInstance{ nullptr };
