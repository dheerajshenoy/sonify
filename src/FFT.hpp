#pragma once

#include "raylib.h"

#include <algorithm>
#include <assert.h>
#include <complex>
#include <vector>

namespace sonify
{

    using complex     = std::complex<double>;
    using vec_complex = std::vector<complex>;

    // In-place FFT, works for power-of-two sizes
    // invert = false -> forward FFT
    // invert = true  -> inverse FFT (divide by N at end)
    static void FFT(vec_complex &a, bool invert = false) noexcept
    {
        const size_t N = a.size();
        assert((N & (N - 1)) == 0 && "FFT size must be power of two!");

        // --- 1) Bit-reversal permutation ---
        for (size_t i = 1, j = 0; i < N; i++)
        {
            size_t bit = N >> 1;
            for (; j & bit; bit >>= 1)
                j ^= bit;
            j ^= bit;
            if (i < j) std::swap(a[i], a[j]);
        }

        // --- 2) Iterative FFT ---
        for (size_t len = 2; len <= N; len <<= 1)
        {
            double ang = 2 * PI / len * (invert ? 1 : -1);
            complex wlen(std::cos(ang), std::sin(ang));
            for (size_t i = 0; i < N; i += len)
            {
                complex w(1);
                for (size_t j = 0; j < len / 2; j++)
                {
                    complex u          = a[i + j];
                    complex v          = a[i + j + len / 2] * w;
                    a[i + j]           = u + v;
                    a[i + j + len / 2] = u - v;
                    w *= wlen;
                }
            }
        }

        // --- 3) Scale for inverse ---
        if (invert)
        {
            for (auto &x : a)
                x /= static_cast<double>(N);
        }
    }

    static void DrawSpectrum(const vec_complex &fft, int screenW, int imageH,
                             int spectrumH, int offsetY) noexcept
    {
        const int bins       = 64; // number of bars to draw
        const int N          = (int)fft.size();
        const size_t fftBins = (size_t)N / 2; // Nyquist limit

        // frequency -> bin mapping
        int binStart = (int)(1.0f * N / 44100.0f);
        int binEnd   = (int)(20000.0f * N / 44100.0f);
        if (binStart < 0) binStart = 0;
        if (binEnd > fftBins) binEnd = fftBins;
        int rangeBins = binEnd - binStart;

        const int binSize    = std::max(1, rangeBins / bins);
        const float barWidth = (float)screenW / bins;

        // compute max magnitude once
        double maxVal = 0.0;
        for (size_t i = 0; i < fftBins; ++i)
        {
            double mag = std::abs(fft.at(i));
            if (mag > maxVal) maxVal = mag;
        }
        if (maxVal == 0.0) maxVal = 1.0; // avoid divide-by-zero

        // baseline for spectrum is just below the image
        int baseY = offsetY + imageH;

        for (size_t i = 0; i < bins; i++)
        {
            double sum = 0;
            for (int j = 0; j < binSize; j++)
            {
                size_t idx = (size_t)i * binSize + j;
                sum += std::abs(fft[idx]);
            }
            double magnitude = sum / binSize;

            // normalize [0..1] against maxVal, then scale to screen height
            double norm   = magnitude / maxVal;
            int barHeight = (int)(norm * spectrumH);

            DrawRectangle((i * barWidth), baseY + spectrumH - barHeight,
                          (int)(barWidth - 2), // spacing
                          barHeight, BLUE);
        }
    }
}; // namespace sonify
