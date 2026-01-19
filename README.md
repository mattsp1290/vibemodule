# Vibemodule - Clouds Reverb DSP

A multi-platform port of the reverb effect from Mutable Instruments Clouds, featuring a Griesinger-topology reverb with modulated delay networks.

## Features

- **High-quality reverb** based on the Mutable Instruments Clouds algorithm
- **Cross-platform DSP core** written in portable C++17
- **Multiple target platforms**:
  - JUCE plugin (VST3/AU) for Windows and macOS
  - Percussa SSP module for ARM Linux
  - Daisy Patch firmware for embedded Cortex-M7

## Parameters

| Parameter | Range | Description |
|-----------|-------|-------------|
| Amount | 0-100% | Dry/wet mix control |
| Input Gain | 0-100% | Pre-reverb gain with soft saturation |
| Time | 0-100% | Reverb decay time |
| Diffusion | 0-100% | Density of reflections (allpass diffuser amount) |
| LP Filter | 0-100% | Lowpass filter cutoff on reverb tail |

## Building

### Prerequisites

- CMake 3.21 or later
- C++17 compatible compiler (GCC 9+, Clang 10+, MSVC 2019+)

### Core Library and Tests

```bash
# Configure
cmake --preset tests

# Build
cmake --build --preset tests

# Run tests
ctest --preset tests
```

### JUCE Plugin

#### Linux Dependencies

```bash
sudo apt-get install -y \
    pkg-config \
    libasound2-dev \
    libfreetype6-dev \
    libx11-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libgl1-mesa-dev \
    libcurl4-openssl-dev \
    libwebkit2gtk-4.0-dev
```

#### Build

```bash
# Configure (Debug)
cmake --preset juce-debug

# Build (Debug)
cmake --build --preset juce-debug

# Or for Release builds:
cmake --preset juce-release
cmake --build --preset juce-release
```

The built plugins will be located in:
- **VST3**: `build/juce-*/platforms/juce/CloudsReverb_artefacts/*/VST3/`
- **AU** (macOS only): `build/juce-*/platforms/juce/CloudsReverb_artefacts/*/AU/`
- **Standalone**: `build/juce-*/platforms/juce/CloudsReverb_artefacts/*/Standalone/`

### Percussa SSP Module

Requires the Percussa SSP SDK and ARM cross-compilation toolchain.

```bash
# Set SSP SDK path
export SSP_SDK_PATH=/path/to/ssp-sdk

# Configure
cmake --preset ssp-release

# Build
cmake --build --preset ssp-release
```

### Daisy Patch Firmware

Requires the ARM GNU Toolchain and libDaisy.

```bash
# Install ARM toolchain (Ubuntu/Debian)
sudo apt-get install gcc-arm-none-eabi

# Clone libDaisy
git clone https://github.com/electro-smith/libDaisy.git
cd libDaisy && make && cd ..

# Build firmware
cd platforms/daisy
make
```

## Project Structure

```
vibemodule/
├── cmake/              # CMake modules (CPM.cmake)
├── libs/
│   └── clouds-dsp/     # Core DSP library
│       └── include/
│           ├── clouds/         # Reverb engine
│           │   ├── clouds_reverb.h
│           │   └── fx_engine.h
│           └── stmlib/         # Ported utilities
│               └── dsp/
├── platforms/
│   ├── juce/           # JUCE plugin
│   ├── ssp/            # Percussa SSP module
│   └── daisy/          # Daisy Patch firmware
└── tests/              # Unit tests (Catch2)
```

## CMake Presets

| Preset | Description |
|--------|-------------|
| `debug` | Core library debug build |
| `release` | Core library release build |
| `tests` | Build with unit tests |
| `juce-debug` | JUCE plugin debug build |
| `juce-release` | JUCE plugin release build |

## Credits

- Original Clouds reverb algorithm by Mutable Instruments (Emilie Gillet)
- Port and multi-platform adaptation by this project

## License

MIT License - see [LICENSE](LICENSE) for details.

The original Clouds source code is released under the MIT License by Mutable Instruments.
