# Sonify

Sonify is a GUI & command-line tool that converts images into sound. It lets you experiment with pixel-to-frequency mappings, spectrums, and headless audio rendering.

> [!NOTE]
> Current version: 0.2.0

# Features

- Map pixel intensities or custom mappings into audio frequencies.
- Load user-defined pixel mappings from shared object (`.so`) plugins.
- Adjustable sample rate, channels, frequency ranges, and traversal modes.
- Optional WAV output.
- Headless mode (no GUI, just audio).
- Live FFT spectrum visualization.

# Installation

> [!WARNING]
> Sonify uses C++20 features and hence requires C++20 or greater compiler to compile

Install `ninja` build system first

```bash
git clone https://github.com/dheerajshenoy/Sonify
cd Sonify
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr # if you want to use Ninja build instead of make
ninja
sudo ninja install
```

This will install Sonify program and also libsonify which is a library used to develop custom pixel mappings.

# Usage

``sonify [options]``

Required arguments

- `--input, -i <file>`
Input file to be sonified (e.g., image).

General options

``--samplerate, -s <float>``
Audio samplerate.
Default: 44100.0

``--channels, -c <int>``
Number of audio channels.
Default: 2

``--output, -o <file>``
Output WAV file name. If not provided, audio is just played.

``--traversal, -t <int>``
Traversal ID to use for scanning the image.
Default: 0

``--background, -b <int>``
Background color (as integer). Ex: `0xFF5000`, `0x000000`
Default: 0x000000

``--pixelmap, -p <string>``
Pixel mapping to use.
Default: Intensity

``--no-spectrum``
Disable FFT spectrum display.

``--headless``
Run without GUI (pure audio/data mode).

``--loop``
Enable audio looping.

``--silent``
Suppress INFO/WARNING messages.

``--dps <float>``
Duration per sample (seconds).
Default: 0.05

``--fmin <float>``
Output minimum frequency.
Default: 0.0

``--fmax <float>``
Output maximum frequency.
Default: 20000.0

``--fps <int>``
Target FPS for GUI rendering.

# Example Commands

Run with defaults:

``sonify -i input.png``

Sonify an image and save to file:

``sonify -i image.png -o output.wav``

Headless audio-only mode:

``sonify -i data.jpg --headless``

Custom samplerate, frequency bounds, and looping:

``sonify -i image.png -s 48000 --fmin 200 --fmax 8000 --loop``

Using a custom mapping:

``sonify -i image.png --pixelmap MyMap``

# Pixel Mappings

Pixel mappings define how pixel values (e.g., RGB, intensity) are converted into audio frequencies.
Sonify ships with some built-in mappings (e.g., Intensity), but you can also write your own.

Custom mappings are loaded from:

``~/.config/sonify/mappings/``

Each mapping is a .so file implementing the MapTemplate interface. At
runtime, Sonify loads all `.so` files in that directory.

> [!NOTE]
> The filename does not matter – only the string returned by name() identifies the mapping.

```cpp
Example: Custom Mapping
#include <sonify/MapTemplate.hpp>
#include <sonify/pixel.hpp>
#include <sonify/utils.hpp>

class MyMapper : public MapTemplate {
public:
    const char* name() const override {
        return "My Custom Mapping";
    }

    std::vector<short>
    mapping(const std::vector<Pixel> &pixelCol) override {
        std::vector<short> wave;
        const int N = pixelCol.size();
        double f = 0;

        for (const auto &px : pixelCol) {
            const HSV hsv = utils::RGBtoHSV(px.rgba);
            f += freq_map(0, 360, _min_freq, _max_freq, hsv.h) /
                 static_cast<double>(N);
        }

        wave = utils::sineWave(0.5, f, 1, _sample_rate);
        return wave;
    }
};

extern "C" MapTemplate* create() {
    return new MyMapper();
}
```

Compile it into a shared object:

``g++ -fPIC -shared MyMapper.cpp -o MyMapper.so``

Use it with:

``sonify -i image.png --pixelmap "My Custom Mapping"``
