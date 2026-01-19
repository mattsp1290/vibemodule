// CloudsReverb - Platform-agnostic wrapper for Clouds Reverb DSP
//
// Based on the reverb effect from Mutable Instruments Clouds
// Original code copyright 2014 Emilie Gillet, MIT License
//
// This wrapper provides:
// - Self-contained memory management (no external buffer needed)
// - Simplified API for common use cases
// - Support for different sample rates
// - Both mono and stereo processing

#ifndef CLOUDS_CLOUDS_REVERB_H_
#define CLOUDS_CLOUDS_REVERB_H_

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <algorithm>

#include "clouds/frame.h"
#include "clouds/fx_engine.h"
#include "stmlib/stmlib.h"

namespace clouds {

// CloudsReverb provides a high-level interface to the Clouds reverb effect.
// It manages its own memory and provides a simple, platform-agnostic API.
class CloudsReverb {
 public:
  // Buffer size for the delay lines (must be power of 2)
  static constexpr size_t kBufferSize = 32768;

  CloudsReverb() {
    std::memset(buffer_, 0, sizeof(buffer_));
  }

  ~CloudsReverb() = default;

  // Initialize the reverb with the given sample rate
  void Init(float sample_rate = 48000.0f) {
    sample_rate_ = sample_rate;
    engine_.Init(buffer_);

    // Scale LFO frequencies for sample rate
    const float rate_scale = 32000.0f / sample_rate_;
    engine_.SetLFOFrequency(LFO_1, 0.5f * rate_scale);
    engine_.SetLFOFrequency(LFO_2, 0.3f * rate_scale);

    // Default parameters
    amount_ = 0.5f;
    input_gain_ = 0.5f;
    reverb_time_ = 0.5f;
    diffusion_ = 0.625f;
    lp_ = 0.7f;
    lp_decay_1_ = 0.0f;
    lp_decay_2_ = 0.0f;
  }

  // Clear all delay buffers (removes any lingering reverb tail)
  void Clear() {
    engine_.Clear();
    lp_decay_1_ = 0.0f;
    lp_decay_2_ = 0.0f;
  }

  // Process stereo audio frames in-place
  void Process(FloatFrame* in_out, size_t size) {
    ProcessInternal(in_out, size);
  }

  // Process separate left/right channel buffers
  void Process(float* left, float* right, size_t size) {
    while (size--) {
      FloatFrame frame = { *left, *right };
      ProcessInternal(&frame, 1);
      *left++ = frame.l;
      *right++ = frame.r;
    }
  }

  // Process mono input to stereo output
  void ProcessMono(const float* input, float* left, float* right, size_t size) {
    while (size--) {
      FloatFrame frame = { *input, *input };
      ProcessInternal(&frame, 1);
      *left++ = frame.l;
      *right++ = frame.r;
      ++input;
    }
  }

  // Parameter setters with range clamping [0.0, 1.0]

  // Wet/dry mix (0 = fully dry, 1 = fully wet)
  void SetAmount(float amount) {
    amount_ = std::clamp(amount, 0.0f, 1.0f);
  }

  // Input gain to the reverb
  void SetInputGain(float input_gain) {
    input_gain_ = std::clamp(input_gain, 0.0f, 1.0f);
  }

  // Reverb decay time (0 = short, 1 = infinite)
  void SetTime(float time) {
    reverb_time_ = std::clamp(time, 0.0f, 1.0f);
  }

  // Diffusion amount (0 = sparse, 1 = dense)
  void SetDiffusion(float diffusion) {
    diffusion_ = std::clamp(diffusion, 0.0f, 1.0f);
  }

  // Low-pass filter cutoff in feedback (0 = dark, 1 = bright)
  void SetLowpassCutoff(float lp) {
    lp_ = std::clamp(lp, 0.0f, 1.0f);
  }

  // Convenience method to set all parameters at once
  void SetParameters(float amount, float input_gain, float time, float diffusion, float lp) {
    SetAmount(amount);
    SetInputGain(input_gain);
    SetTime(time);
    SetDiffusion(diffusion);
    SetLowpassCutoff(lp);
  }

  // Parameter getters
  float GetAmount() const { return amount_; }
  float GetInputGain() const { return input_gain_; }
  float GetTime() const { return reverb_time_; }
  float GetDiffusion() const { return diffusion_; }
  float GetLowpassCutoff() const { return lp_; }
  float GetSampleRate() const { return sample_rate_; }

 private:
  void ProcessInternal(FloatFrame* in_out, size_t size) {
    // Define memory layout for delay lines
    typedef E::Reserve<150,
      E::Reserve<214,
      E::Reserve<319,
      E::Reserve<527,
      E::Reserve<2182,
      E::Reserve<2690,
      E::Reserve<4501,
      E::Reserve<2525,
      E::Reserve<2197,
      E::Reserve<6312> > > > > > > > > > Memory;
    E::DelayLine<Memory, 0> ap1;
    E::DelayLine<Memory, 1> ap2;
    E::DelayLine<Memory, 2> ap3;
    E::DelayLine<Memory, 3> ap4;
    E::DelayLine<Memory, 4> dap1a;
    E::DelayLine<Memory, 5> dap1b;
    E::DelayLine<Memory, 6> del1;
    E::DelayLine<Memory, 7> dap2a;
    E::DelayLine<Memory, 8> dap2b;
    E::DelayLine<Memory, 9> del2;
    E::Context c;

    const float kap = diffusion_;
    const float klp = lp_;
    const float krt = reverb_time_;
    const float amount = amount_;
    const float gain = input_gain_;

    float lp_1 = lp_decay_1_;
    float lp_2 = lp_decay_2_;

    while (size--) {
      float wet;
      float apout = 0.0f;
      engine_.Start(&c);

      // Sum stereo input and apply input gain
      c.Read(in_out->l + in_out->r, gain);

      // 4 input allpass diffusers
      c.Read(ap1 TAIL, kap);
      c.WriteAllPass(ap1, -kap);
      c.Read(ap2 TAIL, kap);
      c.WriteAllPass(ap2, -kap);
      c.Read(ap3 TAIL, kap);
      c.WriteAllPass(ap3, -kap);
      c.Read(ap4 TAIL, kap);
      c.WriteAllPass(ap4, -kap);
      c.Write(apout);

      // Left channel: read from del2, through AP pair, to del1
      c.Load(apout);
      c.Interpolate(del2, 6261.0f, LFO_2, 50.0f, krt);
      c.Lp(lp_1, klp);
      c.Read(dap1a TAIL, -kap);
      c.WriteAllPass(dap1a, kap);
      c.Read(dap1b TAIL, kap);
      c.WriteAllPass(dap1b, -kap);
      c.Write(del1, 2.0f);
      c.Write(wet, 0.0f);

      in_out->l += (wet - in_out->l) * amount;

      // Right channel: read from del1, through AP pair, to del2
      c.Load(apout);
      c.Interpolate(del1, 4460.0f, LFO_1, 40.0f, krt);
      c.Lp(lp_2, klp);
      c.Read(dap2a TAIL, kap);
      c.WriteAllPass(dap2a, -kap);
      c.Read(dap2b TAIL, -kap);
      c.WriteAllPass(dap2b, kap);
      c.Write(del2, 2.0f);
      c.Write(wet, 0.0f);

      in_out->r += (wet - in_out->r) * amount;

      ++in_out;
    }

    lp_decay_1_ = lp_1;
    lp_decay_2_ = lp_2;
  }

  // Use 32-bit float format for desktop platforms
  typedef FxEngine<kBufferSize, FORMAT_32_BIT> E;
  E engine_;
  float buffer_[kBufferSize];

  float sample_rate_;
  float amount_;
  float input_gain_;
  float reverb_time_;
  float diffusion_;
  float lp_;
  float lp_decay_1_;
  float lp_decay_2_;

  DISALLOW_COPY_AND_ASSIGN(CloudsReverb);
};

}  // namespace clouds

#endif  // CLOUDS_CLOUDS_REVERB_H_
