#pragma once

#include "sonify/MapTemplate.hpp"

class HSVMap : public MapTemplate
{
public:

    std::vector<short>
    mapping(const std::vector<Pixel> &pixelCol) noexcept override
    {
        int N    = static_cast<int>(pixelCol.size());
        double f = 0;

        for (const auto &px : pixelCol)
        {
            const HSV hsv = utils::RGBtoHSV(px.rgba);
            f += freq_map(0, 360, _min_freq, _max_freq, hsv.h) /
                 static_cast<double>(N);
        }

        return utils::generateWave(utils::WaveType::SINE, 0.5, f,
                                   _duration_per_sample, _sample_rate);
    }
};
