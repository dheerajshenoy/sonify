#pragma once

#include "sonify/MapTemplate.hpp"
#include "sonify/utils.hpp"

class IntensityMap : public MapTemplate
{
public:

    std::vector<short>
    mapping(const std::vector<Pixel> &pixelCol) noexcept override
    {
        const size_t N = pixelCol.size();
        std::vector<short> fs(N);
        double freq = 0;

        if (N == 0) return {};

        for (const auto &px : pixelCol)
        {
            const HSV hsv = utils::RGBtoHSV(px.rgba);
            freq += freq_map(0, 1, _min_freq, _max_freq, hsv.v);
        }

        utils::generateSineWave(fs, 0.25, freq, _duration_per_sample,
                                _sample_rate);
        utils::applyFadeInOut(fs);
        utils::normalizeWave(fs);
        return fs;
    }
};
