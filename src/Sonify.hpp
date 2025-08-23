#pragma once

#include "DTexture.hpp"
#include "LineItem.hpp"
#include "Pixel.hpp"
#include "utils.hpp"

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
    DTexture *m_texture{ new DTexture() };
    AudioStream m_stream;
    std::vector<Pixel> m_pixelColumn; // temporary column storage
    std::vector<short> m_audioBuffer;
    LineItem *m_li{ new LineItem() };
    bool m_finishedPlayback{ true }, m_audioPlaying{ false };
    int m_audioReadPos{ 0 };
    float m_zoom{ 1.0f };
    void handleMouseScroll() noexcept;
    void handleKeyEvents() noexcept;
    void toggleAudioPlayback() noexcept;
    void setAudioReadPos(int pos) noexcept;
    Camera2D m_camera;
};

static Sonify *gInstance{ nullptr };
