# Clouds Reverb Architecture

This document describes the DSP signal flow and architecture of the Clouds Reverb effect, based on the Griesinger reverb topology.

## Overview

The Clouds Reverb uses a modified Griesinger topology consisting of:

1. **Input diffusion stage** - 4 cascaded allpass filters
2. **Feedback delay network** - 2 parallel branches with modulated delays
3. **Feedback filtering** - Low-pass filters in the feedback path

```
         ┌──────────────────────────────────────────────────────────────────┐
         │                      FEEDBACK NETWORK                             │
         │   ┌─────────┐   ┌─────────┐   ┌─────────┐                        │
Input ───┼──►│ AP1-4   │──►│ Branch1 │──►│ Delay1  │───► Left Output        │
  +      │   │Diffusers│   │ AP+AP   │   │ (mod)   │                        │
Gain     │   └─────────┘   └─────────┘   └────┬────┘                        │
         │        │                           │                              │
         │        │        ┌─────────┐   ┌────▼────┐                        │
         │        └───────►│ Branch2 │──►│ Delay2  │───► Right Output       │
         │                 │ AP+AP   │   │ (mod)   │                        │
         │                 └─────────┘   └────┬────┘                        │
         │                                    │                              │
         └────────────────────────────────────┴──────── Feedback ◄──────────┘
```

## Signal Flow

### Stage 1: Input Processing

```cpp
// Sum stereo input and apply input gain
c.Read(in_out->l + in_out->r, gain);
```

The stereo input is summed to mono and scaled by the input gain parameter. Higher gain values introduce soft saturation.

### Stage 2: Input Diffusion (4 Allpass Filters)

```cpp
// Allpass delay times: 113, 162, 241, 399 samples
c.Read(ap1 TAIL, kap);
c.WriteAllPass(ap1, -kap);
c.Read(ap2 TAIL, kap);
c.WriteAllPass(ap2, -kap);
c.Read(ap3 TAIL, kap);
c.WriteAllPass(ap3, -kap);
c.Read(ap4 TAIL, kap);
c.WriteAllPass(ap4, -kap);
```

Four allpass filters with prime-number delay lengths spread the input impulse in time while preserving total energy. The diffusion coefficient (`kap`) controls how quickly the impulse spreads.

**Allpass Filter Structure:**
```
y[n] = -k*x[n] + x[n-N] + k*y[n-N]
```

Where:
- `k` = diffusion coefficient (0.0-1.0)
- `N` = delay length in samples
- Preserves magnitude while altering phase

### Stage 3: Feedback Delay Network

The feedback network consists of two parallel branches that feed into each other:

#### Left Branch (reads from Delay2, writes to Delay1):

```cpp
c.Load(apout);
c.Interpolate(del2, 4680.0f, LFO_2, 100.0f, krt);  // Modulated read
c.Lp(lp_1, klp);                                     // Low-pass filter
c.Read(dap1a TAIL, -kap);
c.WriteAllPass(dap1a, kap);
c.Read(dap1b TAIL, kap);
c.WriteAllPass(dap1b, -kap);
c.Write(del1, 2.0f);
```

#### Right Branch (reads from Delay1, writes to Delay2):

```cpp
c.Load(apout);
c.Interpolate(del1, 3380.0f, LFO_1, 80.0f, krt);   // Modulated read
c.Lp(lp_2, klp);                                     // Low-pass filter
c.Read(dap2a TAIL, kap);
c.WriteAllPass(dap2a, -kap);
c.Read(dap2b TAIL, -kap);
c.WriteAllPass(dap2b, kap);
c.Write(del2, 2.0f);
```

### Stage 4: Output Mixing

```cpp
// Crossfade between dry and wet
in_out->l += (wet - in_out->l) * amount;
in_out->r += (wet - in_out->r) * amount;
```

## Delay Line Memory Layout

```
Memory (16384 samples total, FORMAT_12_BIT):

┌──────────────────────────────────────────────────────────────────┐
│  Input Allpass Diffusers                                         │
├──────┬──────┬──────┬──────┬──────────────────────────────────────┤
│ AP1  │ AP2  │ AP3  │ AP4  │                                      │
│ 113  │ 162  │ 241  │ 399  │                                      │
├──────┴──────┴──────┴──────┼──────────────────────────────────────┤
│  Feedback Network         │                                      │
├────────┬────────┬─────────┼────────┬────────┬────────────────────┤
│ DAP1a  │ DAP1b  │  DEL1   │ DAP2a  │ DAP2b  │      DEL2          │
│ 1653   │ 2038   │  3411   │ 1913   │ 1663   │      4782          │
└────────┴────────┴─────────┴────────┴────────┴────────────────────┘
```

## LFO Modulation

Two low-frequency oscillators (LFOs) modulate the delay read positions:

| LFO | Frequency | Amplitude | Purpose |
|-----|-----------|-----------|---------|
| LFO_1 | 0.5 Hz | 80 samples | Choruses left delay read |
| LFO_2 | 0.3 Hz | 100 samples | Choruses right delay read |

The LFO values are generated using cosine oscillators and update every 32 samples for efficiency:

```cpp
if ((write_ptr_ & 31) == 0) {
    c->lfo_value_[0] = lfo_[0].Next();
    c->lfo_value_[1] = lfo_[1].Next();
}
```

## One-Pole Low-Pass Filters

The feedback path includes one-pole low-pass filters that simulate high-frequency absorption:

```cpp
inline void Lp(float& state, float coefficient) {
    state += coefficient * (accumulator_ - state);
    accumulator_ = state;
}
```

Higher LP coefficient values allow more high frequencies through, creating a brighter reverb. Lower values create warmer, darker tails.

## FxEngine Template Architecture

The reverb uses a template-based delay engine for efficient memory management:

```cpp
template<size_t size, Format format = FORMAT_12_BIT>
class FxEngine {
    // ...
};
```

### Format Types

| Format | Storage | Range | Precision |
|--------|---------|-------|-----------|
| FORMAT_12_BIT | uint16_t | ±8.0 | ~0.002 |
| FORMAT_16_BIT | uint16_t | ±1.0 | ~0.00003 |
| FORMAT_32_BIT | float | Full | Machine |

The 12-bit format is used for memory efficiency while maintaining acceptable audio quality.

### Static Memory Allocation

Delay line memory is statically allocated using template metaprogramming:

```cpp
typedef Reserve<113,
    Reserve<162,
    Reserve<241,
    Reserve<399, ...>>>> Memory;

DelayLine<Memory, 0> ap1;  // 113 samples at base 0
DelayLine<Memory, 1> ap2;  // 162 samples at base 114
// ...
```

## Griesinger Topology Benefits

The Griesinger topology offers several advantages:

1. **Decorrelation** - Different delay lengths prevent resonant modes
2. **Density** - Multiple allpass stages create dense reflections
3. **Stereo Width** - Cross-fed branches create natural stereo image
4. **Stability** - Proper gain staging prevents feedback runaway
5. **Modulation** - LFO chorus prevents metallic artifacts

## Sample Rate Considerations

The delay lengths are designed for 32kHz sample rate. When running at different sample rates, the LFO frequencies are adjusted:

```cpp
engine_.SetLFOFrequency(LFO_1, 0.5f / sample_rate_);
engine_.SetLFOFrequency(LFO_2, 0.3f / sample_rate_);
```

The delay lengths remain constant in samples, which affects the reverb character:
- At 44.1kHz: Shorter effective delays, tighter reverb
- At 96kHz: Longer effective delays, more spacious reverb

## CloudsReverb Wrapper

The `CloudsReverb` class provides a clean API over the internal `Reverb` class:

```cpp
class CloudsReverb {
public:
    void Init(float sample_rate = 32000.0f);
    void Process(FloatFrame* frames, size_t size);
    void Process(float* left, float* right, size_t size);
    void ProcessMono(float* input, float* left, float* right, size_t size);

    // Parameter setters (0.0-1.0 range, auto-clamped)
    void SetAmount(float amount);
    void SetInputGain(float gain);
    void SetTime(float time);
    void SetDiffusion(float diffusion);
    void SetLowpassCutoff(float lp);
    // ...
};
```

The wrapper handles:
- Memory buffer allocation (32KB internal buffer)
- Parameter clamping and validation
- Sample rate initialization
- Multiple input format conversions
