# Clouds Reverb Parameters

This document describes the parameters available for the Clouds Reverb effect, their ranges, behavior, and musical applications.

## Parameter Overview

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| Amount | 0.0 - 1.0 | 0.5 | Dry/wet mix |
| Input Gain | 0.0 - 1.0 | 0.5 | Pre-reverb gain |
| Time | 0.0 - 1.0 | 0.5 | Reverb decay time |
| Diffusion | 0.0 - 1.0 | 0.625 | Reflection density |
| LP Filter | 0.0 - 1.0 | 0.7 | High frequency damping |

## Detailed Parameter Descriptions

### Amount (Dry/Wet Mix)

**Range:** 0.0 to 1.0 (0% to 100%)
**Default:** 0.5 (50%)

Controls the balance between the original dry signal and the reverb wet signal.

| Value | Effect |
|-------|--------|
| 0.0 | Fully dry - original signal only |
| 0.5 | Equal mix of dry and wet signals |
| 1.0 | Fully wet - reverb only |

**Musical Applications:**
- Use lower values (0.2-0.4) for subtle ambience on lead instruments
- Use 0.5 for a balanced mix suitable for most applications
- Use higher values (0.6-0.8) for ethereal, ambient textures
- Use 1.0 for 100% wet effects processing or parallel sends

### Input Gain

**Range:** 0.0 to 1.0 (0% to 100%)
**Default:** 0.5 (50%)

Controls the level of the signal fed into the reverb algorithm. Higher values introduce soft saturation for a warmer tone.

| Value | Effect |
|-------|--------|
| 0.0-0.3 | Clean, pristine reverb |
| 0.3-0.6 | Normal operation |
| 0.6-0.8 | Warm saturation on transients |
| 0.8-1.0 | Noticeable soft clipping, colored tone |

**Musical Applications:**
- Keep low (0.3-0.5) for clean acoustic recordings
- Increase (0.5-0.7) for drums and percussion to add warmth
- Push higher (0.7-0.9) for vintage, saturated reverb tails
- Useful for adding harmonic content to synth pads

### Time (Reverb Decay)

**Range:** 0.0 to 1.0 (short to infinite)
**Default:** 0.5 (medium decay)

Controls how long the reverb tail persists after the input signal stops. Internally affects the feedback amount in the delay network.

| Value | Approximate Character |
|-------|----------------------|
| 0.0-0.2 | Short room, tight ambience |
| 0.2-0.4 | Small to medium room |
| 0.4-0.6 | Large hall |
| 0.6-0.8 | Cathedral, very long decay |
| 0.8-0.95 | Near-infinite sustain |
| 0.95-1.0 | Infinite reverb (freeze-like) |

**Musical Applications:**
- Short (0.1-0.3) for snare drum and percussion clarity
- Medium (0.3-0.5) for vocals and acoustic instruments
- Long (0.5-0.7) for synth pads and ambient textures
- Very long (0.8+) for drone and experimental soundscapes

**Note:** At very high time values (>0.9), the reverb approaches infinite sustain. This is useful for creating pad-like textures but may cause buildup if the input is continuous.

### Diffusion

**Range:** 0.0 to 1.0 (sparse to dense)
**Default:** 0.625 (moderately dense)

Controls the density of early reflections by adjusting the coefficients of the four input allpass diffusers.

| Value | Effect |
|-------|--------|
| 0.0-0.3 | Sparse reflections, audible echoes |
| 0.3-0.5 | Moderate density, some echo character |
| 0.5-0.7 | Dense, smooth reverb |
| 0.7-0.9 | Very dense, washy texture |
| 0.9-1.0 | Maximum diffusion, smeared transients |

**Musical Applications:**
- Low diffusion (0.2-0.4) for percussive sources where you want rhythm clarity
- Medium diffusion (0.4-0.6) for natural room simulation
- High diffusion (0.6-0.8) for lush pad reverbs
- Very high diffusion (0.8+) for ambient/experimental textures

**Technical Note:** Higher diffusion values cause faster decorrelation of the input signal, resulting in a denser reverb texture. Lower values preserve more of the original transient information.

### LP Filter (Low-Pass Cutoff)

**Range:** 0.0 to 1.0 (dark to bright)
**Default:** 0.7 (moderate brightness)

Controls the cutoff frequency of the low-pass filter in the reverb feedback loop. This simulates the natural high-frequency absorption of real acoustic spaces.

| Value | Effect |
|-------|--------|
| 0.0-0.2 | Very dark, muffled reverb |
| 0.2-0.4 | Dark, warm reverb |
| 0.4-0.6 | Natural room simulation |
| 0.6-0.8 | Bright, present reverb |
| 0.8-1.0 | Very bright, metallic quality |

**Musical Applications:**
- Dark settings (0.2-0.4) for vintage plate-style reverb
- Natural settings (0.4-0.6) for simulating wooden rooms or halls
- Bright settings (0.6-0.8) for modern, clear reverb
- Very bright (0.8+) for special effects or shimmer-type sounds

**Technical Note:** The LP filter coefficient directly affects the high-frequency decay time. With a low LP value, high frequencies decay faster than low frequencies, creating a natural-sounding reverb. With a high LP value, all frequencies decay at similar rates.

## Parameter Interactions

### Time + Diffusion
- High time + low diffusion: Creates evolving echo patterns
- High time + high diffusion: Creates dense, ambient clouds
- Low time + low diffusion: Tight slap-back delay character
- Low time + high diffusion: Short, dense ambience

### Time + LP Filter
- Long time + dark LP: Warm, atmospheric pads
- Long time + bright LP: Shimmering, ethereal textures
- Short time + dark LP: Intimate room sound
- Short time + bright LP: Modern, close-mic ambience

### Input Gain + Amount
- High gain + low amount: Subtle warmth on dry signal
- High gain + high amount: Saturated ambient effects
- Low gain + high amount: Clean, pristine reverb washes

## Preset Starting Points

### Small Room
```
Amount: 0.3
Input Gain: 0.4
Time: 0.2
Diffusion: 0.5
LP Filter: 0.5
```

### Large Hall
```
Amount: 0.5
Input Gain: 0.5
Time: 0.6
Diffusion: 0.625
LP Filter: 0.6
```

### Ambient Pad
```
Amount: 0.8
Input Gain: 0.5
Time: 0.85
Diffusion: 0.8
LP Filter: 0.4
```

### Shimmer
```
Amount: 0.6
Input Gain: 0.6
Time: 0.75
Diffusion: 0.7
LP Filter: 0.9
```

### Vintage Plate
```
Amount: 0.4
Input Gain: 0.6
Time: 0.4
Diffusion: 0.7
LP Filter: 0.35
```
