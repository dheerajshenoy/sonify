// File for creating custom mappings using shared objects
#pragma once

#include "Pixel.hpp"
#include "utils.hpp"

#include <vector>

class MapTemplate
{
public:

    using FreqMapFunc = short (*)(double, double, double, double, double);

    virtual ~MapTemplate()                                         = default;
    virtual std::vector<short> mapping(const std::vector<Pixel> &) = 0;

    inline float minFreq() const noexcept { return _min_freq; }
    inline float maxFreq() const noexcept { return _max_freq; }
    inline float sampleRate() const noexcept { return _sample_rate; }
    inline FreqMapFunc freqMapper() const noexcept { return freq_map; }
    inline float durationPerSample() const noexcept
    {
        return _duration_per_sample;
    }

    inline void setMinFreq(float f) noexcept { _min_freq = f; }
    inline void setMaxFreq(float f) noexcept { _max_freq = f; }
    inline void setSampleRate(float f) noexcept { _sample_rate = f; }
    inline void setFreqMap(FreqMapFunc f) noexcept { freq_map = f; }
    inline void setDurationPerSample(float d) noexcept
    {
        _duration_per_sample = d;
    }

protected:

    FreqMapFunc freq_map{ utils::LinearMap };
    float _min_freq{ 0.0f }, _max_freq{ 20000.0f }, _sample_rate{ 44100.0f },
        _duration_per_sample{ 0.05f };
};

// Factory function for plugins
extern "C"
{
    MapTemplate *create();
}
