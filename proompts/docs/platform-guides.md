# Platform-Specific Development Guides

## JUCE Platform (VST3/AU)

### SDK Setup

JUCE is fetched via CPM. Add to CMakeLists.txt:

```cmake
CPMAddPackage(
    NAME JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.9  # Use latest stable
)
```

### Plugin Structure

```cpp
// PluginProcessor.h
class CloudsReverbProcessor : public juce::AudioProcessor
{
public:
    CloudsReverbProcessor();
    ~CloudsReverbProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    // State management
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

private:
    CloudsReverb reverb;
    juce::AudioProcessorValueTreeState parameters;

    std::atomic<float>* amountParam = nullptr;
    std::atomic<float>* timeParam = nullptr;
    std::atomic<float>* diffusionParam = nullptr;
    std::atomic<float>* lpParam = nullptr;
    std::atomic<float>* inputGainParam = nullptr;
};
```

### CMake Configuration

```cmake
juce_add_plugin(CloudsReverb
    COMPANY_NAME "VibeModule"
    PLUGIN_MANUFACTURER_CODE Vibe
    PLUGIN_CODE Clrv
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "Clouds Reverb"
)

target_link_libraries(CloudsReverb
    PRIVATE
        clouds-dsp
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
)
```

### Build Commands

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --target CloudsReverb_VST3

# macOS: Build AU
cmake --build build --target CloudsReverb_AU
```

---

## Percussa SSP Platform

### SDK Setup

1. Download SSP SDK toolchain:
```bash
wget https://sw13072022.s3.us-west-1.amazonaws.com/arm-rockchip-linux-gnueabihf_sdk-buildroot.tar.gz
tar xzf arm-rockchip-linux-gnueabihf_sdk-buildroot.tar.gz
```

2. Set environment:
```bash
export SSP_SDK=/path/to/ssp-sdk
export PATH=$SSP_SDK/bin:$PATH
```

### Module Structure

```cpp
// CloudsReverbSSP.h
#include "SSP.h"

class CloudsReverbSSP : public SSP::Processor
{
public:
    CloudsReverbSSP();

    // Audio processing
    void process(const float** inputs, float** outputs, int numSamples) override;

    // Parameters
    int getNumParameters() const override { return 5; }
    const char* getParameterName(int index) const override;
    float getParameter(int index) const override;
    void setParameter(int index, float value) override;

    // CV modulation
    int getNumCVInputs() const override { return 4; }
    void processCVInput(int index, float value) override;

private:
    CloudsReverb reverb;
    float params[5];
};
```

### CMake Cross-Compilation

```cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER ${SSP_SDK}/bin/arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER ${SSP_SDK}/bin/arm-linux-gnueabihf-g++)
set(CMAKE_SYSROOT ${SSP_SDK}/arm-rockchip-linux-gnueabihf/sysroot)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=neon -mfloat-abi=hard")
```

### Build Commands

```bash
cmake -B build-ssp -DCMAKE_TOOLCHAIN_FILE=cmake/ssp-toolchain.cmake
cmake --build build-ssp
```

### Installation

Copy built `.so` to SSP's VST folder:
```bash
scp build-ssp/CloudsReverb.so user@ssp:/media/storage/VSTs/
```

---

## Daisy Patch Platform

### SDK Setup

```bash
# Clone libDaisy and DaisySP
git clone --recursive https://github.com/electro-smith/libDaisy.git
git clone https://github.com/electro-smith/DaisySP.git

# Install ARM toolchain
# Ubuntu/Debian:
sudo apt install gcc-arm-none-eabi

# Build libraries
cd libDaisy && make
cd ../DaisySP && make
```

### Application Structure

```cpp
// main.cpp
#include "daisy_patch.h"
#include "clouds/reverb.h"

daisy::DaisyPatch hw;
CloudsReverb reverb;

// Parameter storage
float amount = 0.5f;
float time = 0.7f;
float diffusion = 0.8f;
float lp = 0.6f;

void AudioCallback(daisy::AudioHandle::InputBuffer in,
                   daisy::AudioHandle::OutputBuffer out,
                   size_t size)
{
    // Read controls
    amount = hw.controls[0].Process();
    time = hw.controls[1].Process();
    diffusion = hw.controls[2].Process();
    lp = hw.controls[3].Process();

    // Update parameters
    reverb.set_amount(amount);
    reverb.set_time(time);
    reverb.set_diffusion(diffusion);
    reverb.set_lp(lp);

    // Process audio
    for (size_t i = 0; i < size; i++)
    {
        float left_in = in[0][i];
        float right_in = in[1][i];
        float left_out, right_out;

        reverb.Process(left_in, right_in, &left_out, &right_out);

        out[0][i] = left_out;
        out[1][i] = right_out;
    }
}

int main()
{
    hw.Init();
    hw.SetAudioBlockSize(48);
    hw.SetAudioSampleRate(daisy::SaiHandle::Config::SampleRate::SAI_48KHZ);

    reverb.Init(48000.0f);

    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    while (true)
    {
        hw.ProcessAllControls();
        UpdateDisplay();
        hw.DelayMs(1);
    }
}
```

### Makefile

```makefile
TARGET = clouds_reverb

CPP_SOURCES = main.cpp
CPP_SOURCES += libs/clouds-dsp/src/reverb.cpp

LIBDAISY_DIR = /path/to/libDaisy
DAISYSP_DIR = /path/to/DaisySP

C_INCLUDES += -Ilibs/clouds-dsp/include

include $(LIBDAISY_DIR)/core/Makefile
```

### Build Commands

```bash
make clean
make
make program-dfu  # Upload via USB
```

### Hardware Mapping

| Daisy Patch | Reverb Parameter |
|-------------|------------------|
| Knob 1 (CTRL_1) | Amount (wet/dry) |
| Knob 2 (CTRL_2) | Time (decay) |
| Knob 3 (CTRL_3) | Diffusion |
| Knob 4 (CTRL_4) | LP (brightness) |
| CV In 1 | Amount mod |
| CV In 2 | Time mod |
| CV In 3 | Diffusion mod |
| CV In 4 | LP mod |
| Audio In L/R | Reverb input |
| Audio Out L/R | Reverb output |

---

## File Reservations

When working on each platform, reserve these files:

### Core DSP
```
libs/clouds-dsp/**
tests/dsp/**
```

### JUCE
```
platforms/juce/**
tests/juce/**
```

### SSP
```
platforms/ssp/**
cmake/ssp-toolchain.cmake
```

### Daisy
```
platforms/daisy/**
```
