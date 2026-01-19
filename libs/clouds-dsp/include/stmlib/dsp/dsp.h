// Copyright 2012 Emilie Gillet.
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// Ported to vibemodule - platform-agnostic version

#ifndef STMLIB_DSP_DSP_H_
#define STMLIB_DSP_DSP_H_

#include "stmlib/stmlib.h"
#include <cmath>
#include <algorithm>

namespace stmlib {

// Linear interpolation
inline float Interpolate(const float* table, float index, float size) {
  index *= size;
  CONSTRAIN(index, 0.0f, size - 1.0001f);
  size_t integral = static_cast<size_t>(index);
  float fractional = index - static_cast<float>(integral);
  float a = table[integral];
  float b = table[integral + 1];
  return a + (b - a) * fractional;
}

// Hermite interpolation for higher quality
inline float InterpolateHermite(const float* table, float index, float size) {
  index *= size;
  CONSTRAIN(index, 1.0f, size - 2.0001f);
  size_t integral = static_cast<size_t>(index);
  float fractional = index - static_cast<float>(integral);

  float xm1 = table[integral - 1];
  float x0 = table[integral];
  float x1 = table[integral + 1];
  float x2 = table[integral + 2];

  float c = (x1 - xm1) * 0.5f;
  float v = x0 - x1;
  float w = c + v;
  float a = w + v + (x2 - x0) * 0.5f;
  float b_neg = w + a;

  return ((((a * fractional) - b_neg) * fractional + c) * fractional + x0);
}

// Wrapping interpolation for cyclic tables
inline float InterpolateWrap(const float* table, float index, float size) {
  index -= static_cast<float>(static_cast<int32_t>(index));
  if (index < 0.0f) {
    index += 1.0f;
  }
  index *= size;
  size_t integral = static_cast<size_t>(index);
  float fractional = index - static_cast<float>(integral);
  float a = table[integral];
  float b = table[(integral + 1) % static_cast<size_t>(size)];
  return a + (b - a) * fractional;
}

// Smooth interpolation curve (Hermite/smoothstep)
inline float SmoothStep(float x) {
  return x * x * (3.0f - 2.0f * x);
}

// Crossfade between two values
inline float Crossfade(float a, float b, float fade) {
  return a + (b - a) * fade;
}

// Soft limiting function
inline float SoftLimit(float x) {
  return x * (27.0f + x * x) / (27.0f + 9.0f * x * x);
}

// Soft clipping function (tanh approximation)
inline float SoftClip(float x) {
  if (x < -3.0f) {
    return -1.0f;
  } else if (x > 3.0f) {
    return 1.0f;
  } else {
    return SoftLimit(x);
  }
}

// Clip to 16-bit signed integer range
inline int16_t Clip16(int32_t x) {
  if (x < -32768) {
    return -32768;
  } else if (x > 32767) {
    return 32767;
  }
  return static_cast<int16_t>(x);
}

// Clip to 16-bit unsigned integer range
inline uint16_t ClipU16(int32_t x) {
  if (x < 0) {
    return 0;
  } else if (x > 65535) {
    return 65535;
  }
  return static_cast<uint16_t>(x);
}

// Square root
inline float Sqrt(float x) {
  return std::sqrt(x);
}

// Fast square root approximation (for embedded)
inline float FastSqrt(float x) {
  return std::sqrt(x);  // Use standard sqrt, compiler will optimize
}

// Convert float to 16-bit integer with soft limiting
inline int16_t SoftConvert(float x) {
  return Clip16(static_cast<int32_t>(SoftLimit(x * 0.5f) * 32768.0f));
}

// One-pole low-pass filter coefficient from frequency
inline float OnePoleCoefficient(float frequency, float sample_rate) {
  float w = 2.0f * 3.14159265358979323846f * frequency / sample_rate;
  return 1.0f - std::exp(-w);
}

// One-pole filter macros
#define ONE_POLE(out, in, coefficient) out += (coefficient) * ((in) - out)

#define SLOPE(out, in, positive, negative) \
  { \
    float error = (in) - out; \
    out += (error > 0 ? positive : negative) * error; \
  }

#define SLEW(out, in, delta) \
  { \
    float error = (in) - out; \
    float d = (delta); \
    if (error > d) { \
      error = d; \
    } else if (error < -d) { \
      error = -d; \
    } \
    out += error; \
  }

}  // namespace stmlib

#endif  // STMLIB_DSP_DSP_H_
