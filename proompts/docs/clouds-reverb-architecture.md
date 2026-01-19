# Clouds Reverb Architecture

## Overview

This project ports the reverb effect from the Mutable Instruments Clouds Eurorack module to three platforms: JUCE (VST), Percussa SSP, and Daisy Patch.

## Original Clouds DSP Structure

The Clouds reverb uses a **Griesinger reverb topology**, which is a classic algorithmic reverb design. The implementation is found in `clouds/dsp/fx/reverb.h` in the original repository.

### Key Components

1. **FxEngine** (`clouds/dsp/fx/fx_engine.h`)
   - Template-based delay line management
   - Memory allocation and buffer operations
   - Interpolated reads for modulation

2. **Reverb Class** (`clouds/dsp/fx/reverb.h`)
   - Main reverb processor
   - Griesinger topology implementation

### Signal Flow

```
Input -> Input Gain -> 4x Allpass Diffusers -> Delay Network -> LPF -> Output Mix
                                    ^                |
                                    |________________|
                                    (Feedback with decay)
```

### Griesinger Topology Details

1. **Input Stage**
   - Stereo input summed or processed independently
   - Input gain applied

2. **Diffusion Stage (4 Allpass Filters)**
   - Spreads transients to create initial density
   - Configurable diffusion coefficient

3. **Delay Network**
   - Two nested delay loops
   - Each loop contains 2 allpass pairs
   - Modulated delays create chorusing/shimmer

4. **Feedback Path**
   - Lowpass filter in feedback for natural decay
   - Reverb time controls feedback amount

5. **Output Mix**
   - Wet/dry blend via `amount` parameter

## Parameters

| Parameter | Range | Description |
|-----------|-------|-------------|
| `amount` | 0.0 - 1.0 | Wet/dry mix (0 = dry, 1 = wet) |
| `input_gain` | 0.0 - 1.0 | Input level to reverb |
| `time` | 0.0 - 1.0 | Reverb decay time |
| `diffusion` | 0.0 - 1.0 | Allpass diffuser coefficient |
| `lp` | 0.0 - 1.0 | Lowpass filter cutoff (higher = brighter) |

## Memory Requirements

The reverb uses `FxEngine<16384, FORMAT_12_BIT>` which allocates:
- 16384 samples of delay memory
- 12-bit format for memory efficiency on embedded platforms

For desktop platforms (JUCE), we can use full 32-bit float precision.

## Platform-Specific Considerations

### JUCE (Desktop VST)
- Full 32-bit float precision
- Support 44.1kHz, 48kHz, and 96kHz sample rates
- Smooth parameter changes to avoid zipper noise
- Use JUCE's AudioProcessorValueTreeState for DAW automation

### Percussa SSP
- ARM Cortex-A17 (quad-core 1.8GHz)
- Linux-based, uses VST3 SDK
- CV inputs can modulate parameters
- Design for real-time safety

### Daisy Patch
- ARM Cortex-M7 (480MHz)
- Bare metal, no OS
- More constrained resources
- Consider fixed-point math for optimization
- Map physical knobs to parameters

## File Organization

```
libs/clouds-dsp/
├── CMakeLists.txt
├── include/
│   └── clouds/
│       ├── reverb.h           # Main reverb class
│       ├── fx_engine.h        # Delay engine
│       └── stmlib/            # Math utilities
└── src/
    └── reverb.cpp

platforms/
├── juce/
│   ├── CMakeLists.txt
│   ├── PluginProcessor.cpp
│   ├── PluginProcessor.h
│   ├── PluginEditor.cpp
│   └── PluginEditor.h
├── ssp/
│   ├── CMakeLists.txt
│   ├── CloudsReverbSSP.cpp
│   └── CloudsReverbSSP.h
└── daisy/
    ├── Makefile
    ├── main.cpp
    └── hardware_controls.cpp
```

## References

- [Original Clouds Source](https://github.com/pichenettes/eurorack/tree/master/clouds)
- [JUCE Documentation](https://juce.com/learn/documentation)
- [Percussa SSP SDK](https://github.com/percussa/ssp-sdk)
- [Daisy Wiki](https://github.com/electro-smith/DaisyWiki/wiki)
- [Griesinger Reverb Paper](https://www.aes.org/e-lib/browse.cfm?elib=5163)
