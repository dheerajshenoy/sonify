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
    virtual const char *name() const                               = 0;
    virtual std::vector<short> mapping(const std::vector<Pixel> &) = 0;

    inline int minFreq() const noexcept { return _min_freq; }
    inline int maxFreq() const noexcept { return _max_freq; }
    inline int sampleRate() const noexcept { return _sample_rate; }
    inline FreqMapFunc freqMapper() const noexcept { return freq_map; }

    inline void setMinFreq(int f) noexcept { _min_freq = f; }
    inline void setMaxFreq(int f) noexcept { _max_freq = f; }
    inline void setSampleRate(int f) noexcept { _sample_rate = f; }
    inline void setFreqMap(FreqMapFunc f) noexcept { freq_map = f; }

protected:

    FreqMapFunc freq_map{ utils::LinearMap };
    int _min_freq{ 0 }, _max_freq{ 20000 }, _sample_rate{ 44100 };
};

// Factory function for plugins
extern "C"
{
    MapTemplate *create();
}
