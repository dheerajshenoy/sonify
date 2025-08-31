#include "sonify/utils.hpp"

#include "sonify/Pixel.hpp"

#include <algorithm>
#include <complex>

namespace utils
{

    // Linear mapping
    short LinearMap(double inp_min, double inp_max, double out_min,
                    double out_max, double val) noexcept
    {
        return static_cast<short>(out_min + (val - inp_min) *
                                                (out_max - out_min) /
                                                (inp_max - inp_min));
    }

    // Exponential mapping
    short ExpMap(double inp_min, double inp_max, double out_min, double out_max,
                 double val) noexcept
    {
        double x_norm = (val - inp_min) / (inp_max - inp_min);
        double y_norm = std::pow(x_norm, 2);
        return static_cast<short>(y_norm * (out_max - out_min) + out_min);
    }

    // Logarithm mapping
    short LogMap(double inp_min, double inp_max, double out_min, double out_max,
                 double val) noexcept
    {
        double x_norm = (val - inp_min) / (inp_max - inp_min);
        double y_norm = std::log(x_norm + 1) / std::log(10.0);
        return static_cast<short>(y_norm * (out_max - out_min) + out_min);
    }

    // Apply envelope effect
    void applyEnvelope(std::vector<short> &samples) noexcept
    {

        const int N       = static_cast<int>(samples.size());
        const int attack  = std::min(100, N / 10);
        const int release = std::min(100, N / 10);

        for (int i = 0; i < attack; ++i)
            samples[i] *= static_cast<float>(i) / attack;

        for (int i = 0; i < release; ++i)
            samples[N - 1 - i] *= static_cast<float>(i) / release;
    }

    // Convert hue to frequency
    double Hue2Freq(int H) noexcept
    {
        constexpr double scale_freqs[] = { 220.00, 246.94, 261.63, 293.66,
                                           329.63, 349.23, 415.30 };
        constexpr int thresholds[]     = { 26, 52, 78, 104, 128, 154, 180 };
        auto note                      = scale_freqs[0];

        if (H <= thresholds[0])
            note = scale_freqs[0];

        else if (H > thresholds[0] && H <= thresholds[1])
            note = scale_freqs[1];

        else if (H > thresholds[1] && H <= thresholds[2])
            note = scale_freqs[2];

        else if (H > thresholds[2] && H <= thresholds[3])
            note = scale_freqs[3];

        else if (H > thresholds[3] && H <= thresholds[4])
            note = scale_freqs[4];

        else if (H > thresholds[4] && H <= thresholds[5])
            note = scale_freqs[5];

        else if (H > thresholds[5] && H <= thresholds[6])
            note = scale_freqs[6];

        else
            note = scale_freqs[0];

        return note;
    }

    // Function that is similar to numpy linspace.
    std::vector<double> linspace(double start, double stop, int num) noexcept
    {
        std::vector<double> result;
        result.reserve(num);
        double step = (stop - start) / (num - 1);

        for (int i = 0; i < num; ++i)
        {
            result[i] = start + i * step;
        }

        return result;
    }

    // Applies fade in out to wave
    void applyFadeInOut(std::vector<short> &wave, double fadeFrac) noexcept
    {
        const size_t N  = wave.size();
        size_t fade_len = static_cast<size_t>(N * fadeFrac); // 10% fade

        for (size_t i = 0; i < fade_len; ++i)
        {
            double gain = static_cast<double>(i) / (double)fade_len;
            wave[i] *= static_cast<short>(gain);
            wave[N - 1 - i] *= static_cast<short>(gain);
        }
    }

    // Quantize arbitrary frequency to nearest note in 12-TET scale
    double quantizeToNote(double freq) noexcept
    {
        if (freq <= 0.0) return 440.0;

        double semitones = 12.0 * std::log2(freq / 440.0);
        double rounded   = std::round(semitones);
        return 440.0 * std::pow(2.0, rounded / 12.0);
    }

    // Stereo panning (0.0 = left, 1.0 = right)
    // Expands mono to stereo interleaved L,R
    std::vector<short> panStereo(const std::vector<short> &mono,
                                 float pan) noexcept
    {
        std::vector<short> stereo;
        stereo.reserve(mono.size() * 2);

        float leftGain  = std::cos(pan * M_PI_2); // smooth panning law
        float rightGain = std::sin(pan * M_PI_2);

        for (auto v : mono)
        {
            stereo.push_back(static_cast<short>(v * leftGain));
            stereo.push_back(static_cast<short>(v * rightGain));
        }
        return stereo;
    }

    // Normalizes the wave
    void normalizeWave(std::vector<short> &wave) noexcept
    {
        short max_val = 1;
        for (short v : wave)
            max_val = std::max(max_val, static_cast<short>(std::abs(v)));
        if (max_val == 0 || max_val >= std::numeric_limits<short>::max())
            return;

        double scale =
            std::numeric_limits<short>::max() / static_cast<double>(max_val);
        for (short &v : wave)
            v = static_cast<short>(v * scale);
    }

    float intensity(const RGBA &rgba) noexcept
    {
        return (float)(rgba.r + rgba.g + rgba.b) / 3.0f;
    }

    HSV RGBtoHSV(const RGBA &rgb) noexcept
    {
        double r = rgb.r / 255.0;
        double g = rgb.g / 255.0;
        double b = rgb.b / 255.0;

        double cmax  = std::max({ r, g, b });
        double cmin  = std::min({ r, g, b });
        double delta = cmax - cmin;

        double h = 0.0;

        if (delta == 0)
            h = 0;
        else if (cmax == r)
            h = 60 * (fmod(((g - b) / delta), 6));
        else if (cmax == g)
            h = 60 * (((b - r) / delta) + 2);
        else if (cmax == b)
            h = 60 * (((r - g) / delta) + 4);

        if (h < 0) h += 360; // ensure h is positive

        double s = (cmax == 0) ? 0 : (delta / cmax);
        double v = cmax;

        return { h, s, v };
    }

    std::vector<short> generateWave(WaveType type, double amplitude,
                                    double frequency, double time,
                                    int samplerate) noexcept
    {
        size_t N = static_cast<size_t>(time * samplerate);
        std::vector<short> buffer(N);

        for (size_t n = 0; n < N; ++n)
        {
            double t     = static_cast<double>(n) / samplerate;
            double value = 0.0;

            switch (type)
            {
                case WaveType::SINE: value = sineAt(t, frequency); break;
                case WaveType::SQUARE: value = squareAt(t, frequency); break;
                case WaveType::SAWTOOTH:
                    value = sawtoothAt(t, frequency);
                    break;
                case WaveType::TRIANGLE:
                    value = triangleAt(t, frequency);
                    break;
            }

            buffer[n] = static_cast<short>(amplitude * value * 32767);
        }

        return buffer;
    }

} // namespace utils
