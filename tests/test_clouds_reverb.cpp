#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <clouds/clouds_reverb.h>
#include <cmath>

using Catch::Approx;

TEST_CASE("CloudsReverb initialization", "[reverb]") {
    clouds::CloudsReverb reverb;

    SECTION("Default initialization at 48kHz") {
        reverb.Init(48000.0f);

        CHECK(reverb.GetSampleRate() == 48000.0f);
        CHECK(reverb.GetAmount() == Approx(0.5f));
        CHECK(reverb.GetInputGain() == Approx(0.5f));
        CHECK(reverb.GetTime() == Approx(0.5f));
        CHECK(reverb.GetDiffusion() == Approx(0.625f));
        CHECK(reverb.GetLowpassCutoff() == Approx(0.7f));
    }

    SECTION("Initialization at 44.1kHz") {
        reverb.Init(44100.0f);
        CHECK(reverb.GetSampleRate() == 44100.0f);
    }

    SECTION("Initialization at 96kHz") {
        reverb.Init(96000.0f);
        CHECK(reverb.GetSampleRate() == 96000.0f);
    }
}

TEST_CASE("CloudsReverb parameter clamping", "[reverb]") {
    clouds::CloudsReverb reverb;
    reverb.Init();

    SECTION("Amount clamping") {
        reverb.SetAmount(-0.5f);
        CHECK(reverb.GetAmount() == 0.0f);

        reverb.SetAmount(1.5f);
        CHECK(reverb.GetAmount() == 1.0f);

        reverb.SetAmount(0.75f);
        CHECK(reverb.GetAmount() == Approx(0.75f));
    }

    SECTION("Time clamping") {
        reverb.SetTime(-1.0f);
        CHECK(reverb.GetTime() == 0.0f);

        reverb.SetTime(2.0f);
        CHECK(reverb.GetTime() == 1.0f);
    }
}

TEST_CASE("CloudsReverb SetParameters convenience method", "[reverb]") {
    clouds::CloudsReverb reverb;
    reverb.Init();

    reverb.SetParameters(0.3f, 0.4f, 0.5f, 0.6f, 0.8f);

    CHECK(reverb.GetAmount() == Approx(0.3f));
    CHECK(reverb.GetInputGain() == Approx(0.4f));
    CHECK(reverb.GetTime() == Approx(0.5f));
    CHECK(reverb.GetDiffusion() == Approx(0.6f));
    CHECK(reverb.GetLowpassCutoff() == Approx(0.8f));
}

TEST_CASE("CloudsReverb stereo processing", "[reverb]") {
    clouds::CloudsReverb reverb;
    reverb.Init(48000.0f);
    reverb.SetAmount(1.0f);  // Full wet
    reverb.SetTime(0.5f);

    // Create impulse
    constexpr size_t bufferSize = 512;
    clouds::FloatFrame frames[bufferSize] = {};
    frames[0].l = 1.0f;
    frames[0].r = 1.0f;

    // Process
    reverb.Process(frames, bufferSize);

    // After processing an impulse, we should have some reverb output
    // Check that output is not just the original impulse (energy is spread)
    float energySum = 0.0f;
    for (size_t i = 1; i < bufferSize; ++i) {
        energySum += frames[i].l * frames[i].l + frames[i].r * frames[i].r;
    }

    // There should be reverb tail energy
    CHECK(energySum > 0.001f);
}

TEST_CASE("CloudsReverb separate channel processing", "[reverb]") {
    clouds::CloudsReverb reverb;
    reverb.Init(48000.0f);
    reverb.SetAmount(0.5f);

    constexpr size_t bufferSize = 256;
    float left[bufferSize] = {};
    float right[bufferSize] = {};
    left[0] = 1.0f;
    right[0] = 1.0f;

    reverb.Process(left, right, bufferSize);

    // Check that processing happened
    float energySum = 0.0f;
    for (size_t i = 1; i < bufferSize; ++i) {
        energySum += left[i] * left[i] + right[i] * right[i];
    }
    CHECK(energySum > 0.0001f);
}

TEST_CASE("CloudsReverb mono to stereo processing", "[reverb]") {
    clouds::CloudsReverb reverb;
    reverb.Init(48000.0f);
    reverb.SetAmount(1.0f);

    constexpr size_t bufferSize = 256;
    float mono[bufferSize] = {};
    float left[bufferSize] = {};
    float right[bufferSize] = {};
    mono[0] = 1.0f;

    reverb.ProcessMono(mono, left, right, bufferSize);

    // Check stereo output from mono input
    CHECK(left[0] != 0.0f);
    CHECK(right[0] != 0.0f);
}

TEST_CASE("CloudsReverb Clear resets state", "[reverb]") {
    clouds::CloudsReverb reverb;
    reverb.Init(48000.0f);
    reverb.SetAmount(1.0f);

    // Process some audio
    constexpr size_t bufferSize = 256;
    clouds::FloatFrame frames[bufferSize] = {};
    frames[0].l = 1.0f;
    frames[0].r = 1.0f;
    reverb.Process(frames, bufferSize);

    // Clear
    reverb.Clear();

    // Process silence - output should be very close to zero
    clouds::FloatFrame silentFrames[bufferSize] = {};
    reverb.Process(silentFrames, bufferSize);

    float energySum = 0.0f;
    for (size_t i = 0; i < bufferSize; ++i) {
        energySum += silentFrames[i].l * silentFrames[i].l + silentFrames[i].r * silentFrames[i].r;
    }

    // After clear, processing silence should produce near-zero output
    CHECK(energySum < 0.0001f);
}

TEST_CASE("CloudsReverb dry signal passthrough", "[reverb]") {
    clouds::CloudsReverb reverb;
    reverb.Init(48000.0f);
    reverb.SetAmount(0.0f);  // Full dry

    constexpr size_t bufferSize = 64;
    clouds::FloatFrame frames[bufferSize];
    for (size_t i = 0; i < bufferSize; ++i) {
        frames[i].l = static_cast<float>(i) / bufferSize;
        frames[i].r = -static_cast<float>(i) / bufferSize;
    }

    // Save original values
    float originalL[bufferSize];
    float originalR[bufferSize];
    for (size_t i = 0; i < bufferSize; ++i) {
        originalL[i] = frames[i].l;
        originalR[i] = frames[i].r;
    }

    reverb.Process(frames, bufferSize);

    // With amount=0, output should equal input
    for (size_t i = 0; i < bufferSize; ++i) {
        CHECK(frames[i].l == Approx(originalL[i]).margin(0.0001f));
        CHECK(frames[i].r == Approx(originalR[i]).margin(0.0001f));
    }
}
