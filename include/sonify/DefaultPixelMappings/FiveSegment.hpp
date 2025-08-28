#pragma once

#include "sonify/MapTemplate.hpp"
#include "sonify/utils.hpp"

#include <print>

class FiveSegmentMap : public MapTemplate
{
public:

    std::vector<short>
    mapping(const std::vector<Pixel> &pixelCol) noexcept override
    {
        const size_t N = pixelCol.size();
        if (N == 0) return {};

        std::vector<short> result(N, 0);

        const int nSegments     = 5;
        const int segmentHeight = N / nSegments;

        for (int i = 0; i < nSegments; i++)
        {
            double freq = 0.0;
            for (size_t j = 0; j < segmentHeight; j++)
            {
                const Pixel pixel = pixelCol.at(i * segmentHeight + j);
                const HSV hsv     = utils::RGBtoHSV(pixel.rgba);

                const double mapped =
                    freq_map(0, 1000, _min_freq, 4000, pixel.x / (pixel.y + 1));

                freq += utils::quantizeToNote(mapped);
                // freq += freq_map(0, 360, _min_freq, _max_freq, hsv.h);
            }

            freq /= segmentHeight;

            auto fs =
                utils::sineWave(0.25, freq, _duration_per_sample, _sample_rate);

            auto fs2 = utils::sineWave(0.1, freq * 2, _duration_per_sample,
                                       _sample_rate);

            auto fs3 = utils::sineWave(0.05, freq * 3, _duration_per_sample,
                                       _sample_rate);

            fs = utils::addVectors(fs, utils::addVectors(fs2, fs3));

            // Apply envelope for smoothness
            utils::applyFadeInOut(fs);

            // Pan alternating segments left/right if stereo
            if (i % 2 == 0)
                utils::panStereo(fs, 0.8f); // left
            else
                utils::panStereo(fs, 0.2f); // right

            result = utils::addVectors(result, fs);
        }

        utils::normalizeWave(result);

        return result;
    }
};
