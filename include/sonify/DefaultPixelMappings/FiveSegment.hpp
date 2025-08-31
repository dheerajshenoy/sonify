#pragma once

#include "sonify/MapTemplate.hpp"
#include "sonify/utils.hpp"

#include <cmath>
#include <print>

class FiveSegmentMap : public MapTemplate
{
public:

    std::vector<short>
    mapping(const std::vector<Pixel> &pixelCol) noexcept override
    {
        const size_t N =
            static_cast<size_t>(_duration_per_sample * _sample_rate);
        if (N == 0 || pixelCol.empty()) return {};

        std::vector<short> fs(N, 0);

        const int nSegments = 5;
        const int segmentHeight =
            std::max(1, static_cast<int>(pixelCol.size() / nSegments));
        std::vector<std::pair<float, float>> harmonics = { { 0.5f, 1.0f },
                                                           { 0.4f, 3.0f },
                                                           { 0.3f, 5.0f },
                                                           { 0.2f, 6.0f },
                                                           { 0.1f, 7.0f } };

        for (int seg = 0; seg < nSegments; ++seg)
        {
            double freq = 0.0;
            int count   = 0;
            for (int j = 0;
                 j < segmentHeight && seg * segmentHeight + j < pixelCol.size();
                 ++j)
            {
                const Pixel &pixel = pixelCol[seg * segmentHeight + j];
                const RGBA rgba    = pixel.rgba;
                double mapped =
                    freq_map(0, 1000, _min_freq, _max_freq,
                             (rgba.r + rgba.g + rgba.b + rgba.a) / 4);
                freq += utils::quantizeToNote(mapped);
                ++count;
            }
            if (count > 0) freq /= count;

            for (size_t i = 0; i < N; ++i)
            {
                double sample = 0.0;
                for (auto [amp, mult] : harmonics)
                    sample += amp * std::sin(2.0 * M_PI * freq * mult * i /
                                             _sample_rate);

                // Optional simple envelope
                float env = 1.0f;
                if (i < 64)
                    env = i / 64.0f;
                else if (i > N - 64)
                    env = (N - i) / 64.0f;

                // Pan alternating segments
                float pan = (seg % 2 == 0) ? 0.8f : 0.2f;
                sample *= env * pan;

                fs[i] += static_cast<short>(sample * 32767.0);
            }
        }

        // Normalize overall
        utils::normalizeWave(fs);
        return fs;
    }
};
