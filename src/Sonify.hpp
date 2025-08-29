#pragma once

#include "CircleItem.hpp"
#include "DTexture.hpp"
#include "FFT.hpp"
#include "LineItem.hpp"
#include "PathItem.hpp"
#include "PixelMapManager.hpp"
#include "Timer.hpp"
#include "argparse.hpp"
#include "raylib.h"
#include "sonify/DefaultPixelMappings/FiveSegment.hpp"
#include "sonify/DefaultPixelMappings/HSVMap.hpp"
#include "sonify/DefaultPixelMappings/IntensityMap.hpp"
#include "sonify/Pixel.hpp"
#include "sonify/utils.hpp"
#include "toml.hpp"

#include <fftw3.h>
#include <functional>
#include <mutex>
#include <print>
#include <string>

#define LOG(...)         std::println(__VA_ARGS__);
#define __SONIFY_VERSION "0.2.0"

class Sonify
{
public:

    Sonify(const argparse::ArgumentParser &) noexcept;
    ~Sonify() noexcept;

    [[nodiscard("check error status")]] bool
    OpenImage(std::string fileName) noexcept;

private:

    static void audioCallback(void *bufferData, unsigned int frames);

    void sonification() noexcept;
    void GUIloop() noexcept;
    void render() noexcept;
    void handleMouseScroll() noexcept;
    void handleMouseEvents() noexcept;
    void handleKeyEvents() noexcept;
    void toggleAudioPlayback() noexcept;
    void updateCursorUpdater() noexcept;
    [[nodiscard("Get returned string")]] std::string
    replaceHome(const std::string_view &str) noexcept;

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
    void renderFFT() noexcept;
    void parse_args(const argparse::ArgumentParser &) noexcept;
    void setSamplerate(float SR) noexcept;
    void recenterView() noexcept;
    void centerImage() noexcept;
    void seekCursor(float seconds) noexcept;
    bool saveAudio(const std::string &fileName) noexcept;
    void loadUserPixelMappings() noexcept;
    void loadPixelMappingsSharedObjectsFromDir(const std::string &dir) noexcept;
    void loadPixelMappingsSharedObject(const std::string &path) noexcept;
    void loadDefaultPixelMappings() noexcept;
    [[nodiscard]] constexpr Color ColorFromHex(unsigned int hex) noexcept
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
    void readConfigFile() noexcept;
    bool renderVideo() noexcept;
    void reloadCurrentPixelMappingSharedObject() noexcept;

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

    enum class SaveType
    {
        NONE = 0,
        AUDIO_VIDEO,
        AUDIO_ONLY,
        VIDEO_ONLY
    };

    SaveType m_saveType{ SaveType::NONE };

    // used to store the file name to be opened through the command
    // line argument as the GUI loop is still not opened yet
    std::string m_openFileNameRequested;

    std::string m_pixelMapName{ "Intensity" };
    LineItem *m_li{ nullptr };
    CircleItem *m_ci{ nullptr };
    PathItem *m_pi{ nullptr };

    bool m_finishedPlayback{ true };
    bool m_audioPlaying{ false };
    bool m_isSonified{ false };
    bool m_showNotSonifiedMessage{ false };
    bool m_showDragDropText{ true };
    bool m_exit_requested{ false };
    bool m_isAudioSaved{ false };
    bool m_isVideoSaved{ false };
    bool m_isVideoRendering{ false };

    float m_showNotSonifiedMessageTimer{ 1.5f };
    unsigned int m_audioReadPos{ 0 };

    Camera2D m_camera;
    int m_screenW, m_screenH;
    PixelMapManager *m_pixelMapManager{ nullptr };

    std::string m_dragDropText{ "Drop an image file here to sonify" };
    const std::string m_mappings_dir =
        replaceHome("~/.config/sonify/mappings/");
    Color m_dragDropTextColor{ DARKGRAY };
    Font m_font;
    std::string m_font_family;
    int m_font_size{ 60 };
    Timer m_timer;
    unsigned int m_window_config_flags;
    RenderTexture2D m_recordTarget{};
    FILE *m_ffmpeg{ nullptr };

    std::mutex m_reloadMutex;

    // COMMAND LINE ARGUMENTS
    TraversalType m_traversal_type{ 0 };
    std::array<int, 2> m_resize_array{ -1, -1 };
    float m_min_freq{ 0 };
    float m_max_freq{ 20000 };
    float m_sampleRate{ 44100.0f };
    unsigned int m_channels{ 1 };
    unsigned int m_fps{ 60 };
    MapTemplate::FreqMapFunc m_freq_map_func{ utils::LinearMap };
    Color m_bg{ ColorFromHex(0x000000) };
    bool m_display_fft_spectrum{ true };
    bool m_headless{ false };
    bool m_loop{ false };
    float m_duration_per_sample{ 0.05f };
    bool m_silence{ false }; // handles displaying INFO/WARNING messages
    unsigned int m_cursor_thickness{ 1 };
};

static Sonify *gInstance{ nullptr };
