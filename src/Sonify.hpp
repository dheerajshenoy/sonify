#pragma once

#include "CircleItem.hpp"
#include "DTexture.hpp"
#include "LineItem.hpp"
#include "PathItem.hpp"
#include "Pixel.hpp"

#include <fftw3.h>
#include <functional>
#include <raylib.h>
#include <string>

#define LOG(...) std::println(__VA_ARGS__);

class Sonify
{
public:

    Sonify() noexcept;
    ~Sonify() noexcept;

    void OpenImage(const std::string &fileName) noexcept;

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

    using CursorUpdater = std::function<void(int pos)>;
    CursorUpdater m_cursorUpdater;

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
    bool m_finishedPlayback{ true }, m_audioPlaying{ false };
    int m_audioReadPos{ 0 };

    TraversalType m_traversal_type{ 0 };
    Camera2D m_camera;
};

static Sonify *gInstance{ nullptr };
