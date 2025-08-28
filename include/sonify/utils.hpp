#pragma once

#include "Pixel.hpp"

#include <vector>

namespace utils
{
    // ------- Math and mapping -------
    short LinearMap(double value, double in_min, double in_max, double out_min,
                    double out_max) noexcept;

    short ExpMap(double value, double in_min, double in_max, double out_min,
                 double out_max) noexcept;

    short LogMap(double value, double in_min, double in_max, double out_min,
                 double out_max) noexcept;

    // ------- Signal generation -------
    void generateSineWave(std::vector<short> &buffer, double amplitude,
                          double frequency, double time,
                          int samplerate) noexcept;
    std::vector<short> sineWave(double _amplitude, double frequency,
                                double time, float samplerate) noexcept;

    // ------- Signal Effects --------
    void applyEnvelope(std::vector<short> &samples) noexcept;
    void normalizeWave(std::vector<short> &wave) noexcept;
    void applyFadeInOut(std::vector<short> &wave,
                        double fadeFrac = 0.05) noexcept;
    std::vector<short> panStereo(const std::vector<short> &mono,
                                 float pan) noexcept;
    // Quantize arbitrary frequency to nearest note in 12-TET scale
    double quantizeToNote(double freq) noexcept;

    // ------- Utility -------
    double Hue2Freq(int hue) noexcept;
    std::vector<double> linspace(double start, double end, int num) noexcept;
    HSV RGBtoHSV(const RGBA &) noexcept;
    float intensity(const RGBA &) noexcept;

    // Vector utils

    template <typename T>
    std::vector<T> addTwoVectors(const std::vector<T> &first,
                                 const std::vector<T> &second) noexcept
    {
        std::vector<T> result;
        result.reserve(first.size());
        for (size_t i = 0; i < first.size(); ++i)
            result.push_back(first[i] + second[i]);
        return result;
    }

    // Base case: just return one vector
    template <typename T>
    std::vector<T> addVectors(const std::vector<T> &v) noexcept
    {
        return v;
    }

    template <typename T, typename... Args>
    std::vector<T> addVectors(const std::vector<T> &first,
                              const std::vector<T> &second,
                              const Args &...args) noexcept
    {
        return addVectors(addTwoVectors(first, second), args...);
    }

} // namespace utils
