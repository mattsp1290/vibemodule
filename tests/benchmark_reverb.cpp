#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <clouds/clouds_reverb.h>
#include <cmath>

constexpr size_t kBenchmarkSampleRate = 48000;
constexpr size_t kBenchmarkBlockSize = 512;
constexpr size_t kBenchmarkNumBlocks = 100;

TEST_CASE("CloudsReverb performance benchmarks", "[benchmark][reverb]") {
    clouds::CloudsReverb reverb;
    reverb.Init(static_cast<float>(kBenchmarkSampleRate));

    // Prepare test buffers
    std::vector<float> left(kBenchmarkBlockSize);
    std::vector<float> right(kBenchmarkBlockSize);

    // Fill with test signal (simple sine wave)
    for (size_t i = 0; i < kBenchmarkBlockSize; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(kBenchmarkSampleRate);
        left[i] = std::sin(2.0f * 3.14159265f * 440.0f * t) * 0.5f;
        right[i] = std::sin(2.0f * 3.14159265f * 440.0f * t + 0.1f) * 0.5f;
    }

    BENCHMARK("Process 512 samples (stereo)") {
        reverb.Process(left.data(), right.data(), kBenchmarkBlockSize);
        return left[0] + right[0];  // Prevent optimization
    };

    BENCHMARK("Process 512 samples at different Time settings") {
        reverb.SetTime(0.9f);  // Long reverb time
        reverb.Process(left.data(), right.data(), kBenchmarkBlockSize);
        reverb.SetTime(0.1f);  // Short reverb time
        reverb.Process(left.data(), right.data(), kBenchmarkBlockSize);
        return left[0] + right[0];
    };
}

TEST_CASE("CloudsReverb per-sample performance", "[benchmark][reverb]") {
    clouds::CloudsReverb reverb;
    reverb.Init(static_cast<float>(kBenchmarkSampleRate));

    // Test with different buffer sizes to measure per-sample overhead
    for (size_t blockSize : {32, 64, 128, 256, 512, 1024}) {
        std::vector<float> left(blockSize);
        std::vector<float> right(blockSize);

        // Fill with noise-like signal
        for (size_t i = 0; i < blockSize; ++i) {
            left[i] = (static_cast<float>(i % 17) / 17.0f - 0.5f);
            right[i] = (static_cast<float>(i % 13) / 13.0f - 0.5f);
        }

        BENCHMARK("Process " + std::to_string(blockSize) + " samples") {
            reverb.Process(left.data(), right.data(), blockSize);
            return left[0];
        };
    }
}

TEST_CASE("CloudsReverb initialization benchmark", "[benchmark][reverb]") {
    BENCHMARK("Init at 48kHz") {
        clouds::CloudsReverb reverb;
        reverb.Init(48000.0f);
        return reverb.GetSampleRate();
    };

    BENCHMARK("Init at 44.1kHz") {
        clouds::CloudsReverb reverb;
        reverb.Init(44100.0f);
        return reverb.GetSampleRate();
    };

    BENCHMARK("Init at 96kHz") {
        clouds::CloudsReverb reverb;
        reverb.Init(96000.0f);
        return reverb.GetSampleRate();
    };
}

TEST_CASE("CloudsReverb Clear benchmark", "[benchmark][reverb]") {
    clouds::CloudsReverb reverb;
    reverb.Init(static_cast<float>(kBenchmarkSampleRate));

    // Fill with some signal first
    std::vector<float> left(kBenchmarkBlockSize, 1.0f);
    std::vector<float> right(kBenchmarkBlockSize, 1.0f);
    reverb.Process(left.data(), right.data(), kBenchmarkBlockSize);

    BENCHMARK("Clear reverb state") {
        reverb.Clear();
        return 0;
    };
}

TEST_CASE("CloudsReverb parameter setting benchmark", "[benchmark][reverb]") {
    clouds::CloudsReverb reverb;
    reverb.Init(static_cast<float>(kBenchmarkSampleRate));

    BENCHMARK("SetAmount") {
        reverb.SetAmount(0.5f);
        return reverb.GetAmount();
    };

    BENCHMARK("SetTime") {
        reverb.SetTime(0.7f);
        return reverb.GetTime();
    };

    BENCHMARK("SetParameters (all 5)") {
        reverb.SetParameters(0.5f, 0.5f, 0.7f, 0.625f, 0.7f);
        return reverb.GetAmount();
    };
}

TEST_CASE("CloudsReverb FloatFrame processing benchmark", "[benchmark][reverb]") {
    clouds::CloudsReverb reverb;
    reverb.Init(static_cast<float>(kBenchmarkSampleRate));

    std::vector<clouds::FloatFrame> frames(kBenchmarkBlockSize);
    for (size_t i = 0; i < kBenchmarkBlockSize; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(kBenchmarkSampleRate);
        frames[i].l = std::sin(2.0f * 3.14159265f * 440.0f * t) * 0.5f;
        frames[i].r = std::sin(2.0f * 3.14159265f * 440.0f * t + 0.1f) * 0.5f;
    }

    BENCHMARK("Process 512 frames (FloatFrame interface)") {
        reverb.Process(frames.data(), kBenchmarkBlockSize);
        return frames[0].l + frames[0].r;
    };
}

TEST_CASE("CloudsReverb mono to stereo benchmark", "[benchmark][reverb]") {
    clouds::CloudsReverb reverb;
    reverb.Init(static_cast<float>(kBenchmarkSampleRate));

    std::vector<float> mono(kBenchmarkBlockSize);
    std::vector<float> left(kBenchmarkBlockSize);
    std::vector<float> right(kBenchmarkBlockSize);

    for (size_t i = 0; i < kBenchmarkBlockSize; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(kBenchmarkSampleRate);
        mono[i] = std::sin(2.0f * 3.14159265f * 440.0f * t) * 0.5f;
    }

    BENCHMARK("ProcessMono 512 samples") {
        reverb.ProcessMono(mono.data(), left.data(), right.data(), kBenchmarkBlockSize);
        return left[0] + right[0];
    };
}
