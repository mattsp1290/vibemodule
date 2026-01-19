#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <clouds/fx_engine.h>
#include <cmath>

using Catch::Approx;

// Test with 32-bit format for easier value verification
constexpr size_t kTestBufferSize = 256;
using TestEngine = clouds::FxEngine<kTestBufferSize, clouds::FORMAT_32_BIT>;

// Memory layout for test delay lines
struct TestMemory {
    using Line1 = TestEngine::Reserve<64>;
    using Line2 = TestEngine::Reserve<32, Line1>;
    using Line3 = TestEngine::Reserve<16, Line2>;
};

TEST_CASE("FxEngine initialization", "[fxengine]") {
    TestEngine engine;
    float buffer[kTestBufferSize] = {};

    SECTION("Buffer initialization") {
        // Fill buffer with non-zero values
        for (size_t i = 0; i < kTestBufferSize; ++i) {
            buffer[i] = 1.0f;
        }

        engine.Init(buffer);

        // Init should clear the buffer
        for (size_t i = 0; i < kTestBufferSize; ++i) {
            CHECK(buffer[i] == 0.0f);
        }
    }

    SECTION("Clear resets buffer") {
        engine.Init(buffer);

        // Write some data
        TestEngine::Context ctx;
        engine.Start(&ctx);
        ctx.Load(0.5f);

        // Clear
        engine.Clear();

        // Buffer should be zero
        for (size_t i = 0; i < kTestBufferSize; ++i) {
            CHECK(buffer[i] == 0.0f);
        }
    }
}

TEST_CASE("FxEngine Context basic operations", "[fxengine][context]") {
    TestEngine engine;
    float buffer[kTestBufferSize] = {};
    engine.Init(buffer);

    TestEngine::Context ctx;
    engine.Start(&ctx);

    SECTION("Load sets accumulator") {
        ctx.Load(0.75f);
        float value = 0.0f;
        ctx.Write(value);
        CHECK(value == Approx(0.75f));
    }

    SECTION("Read adds to accumulator") {
        ctx.Load(0.5f);
        ctx.Read(0.25f);
        float value = 0.0f;
        ctx.Write(value);
        CHECK(value == Approx(0.75f));
    }

    SECTION("Read with scale") {
        ctx.Load(0.0f);
        ctx.Read(1.0f, 0.5f);
        float value = 0.0f;
        ctx.Write(value);
        CHECK(value == Approx(0.5f));
    }

    SECTION("Write with scale") {
        ctx.Load(1.0f);
        float value = 0.0f;
        ctx.Write(value, 0.5f);
        CHECK(value == Approx(1.0f));

        // Accumulator should be scaled
        float value2 = 0.0f;
        ctx.Write(value2);
        CHECK(value2 == Approx(0.5f));
    }
}

TEST_CASE("FxEngine delay line write and read", "[fxengine][delay]") {
    TestEngine engine;
    float buffer[kTestBufferSize] = {};
    engine.Init(buffer);

    using DL = TestEngine::DelayLine<TestMemory::Line1, 0>;
    DL delay_line;

    SECTION("Write to delay line") {
        TestEngine::Context ctx;
        engine.Start(&ctx);
        ctx.Load(0.8f);
        ctx.Write(delay_line, 0, 1.0f);

        // Value should be in buffer
        bool found = false;
        for (size_t i = 0; i < kTestBufferSize; ++i) {
            if (buffer[i] != 0.0f) {
                found = true;
                CHECK(buffer[i] == Approx(0.8f));
                break;
            }
        }
        CHECK(found);
    }

    SECTION("Write and read back") {
        // Write a value
        TestEngine::Context ctx1;
        engine.Start(&ctx1);
        ctx1.Load(0.5f);
        ctx1.Write(delay_line, 0, 1.0f);

        // Advance time and read
        TestEngine::Context ctx2;
        engine.Start(&ctx2);
        ctx2.Load(0.0f);
        ctx2.Read(delay_line, 1, 1.0f);  // Read from offset 1 to get previous write

        float value = 0.0f;
        ctx2.Write(value);
        CHECK(value == Approx(0.5f));
    }
}

TEST_CASE("FxEngine low-pass filter", "[fxengine][filter]") {
    TestEngine engine;
    float buffer[kTestBufferSize] = {};
    engine.Init(buffer);

    SECTION("LP filter smooths signal") {
        float state = 0.0f;
        float coefficient = 0.5f;

        TestEngine::Context ctx;
        engine.Start(&ctx);
        ctx.Load(1.0f);
        ctx.Lp(state, coefficient);

        float value = 0.0f;
        ctx.Write(value);

        CHECK(state == Approx(0.5f));
        CHECK(value == Approx(0.5f));
    }

    SECTION("LP filter converges to input") {
        float state = 0.0f;
        float coefficient = 0.5f;

        // Process multiple samples toward target of 1.0
        for (int i = 0; i < 10; ++i) {
            TestEngine::Context ctx;
            engine.Start(&ctx);
            ctx.Load(1.0f);
            ctx.Lp(state, coefficient);
        }

        // Should be very close to 1.0 after multiple iterations
        CHECK(state > 0.99f);
    }
}

TEST_CASE("FxEngine high-pass filter", "[fxengine][filter]") {
    TestEngine engine;
    float buffer[kTestBufferSize] = {};
    engine.Init(buffer);

    SECTION("HP filter removes DC") {
        float state = 0.0f;
        float coefficient = 0.1f;

        // Feed constant DC
        for (int i = 0; i < 100; ++i) {
            TestEngine::Context ctx;
            engine.Start(&ctx);
            ctx.Load(1.0f);
            ctx.Hp(state, coefficient);
        }

        // After convergence, HP output should be near zero for DC input
        TestEngine::Context ctx;
        engine.Start(&ctx);
        ctx.Load(1.0f);
        ctx.Hp(state, coefficient);

        float value = 0.0f;
        ctx.Write(value);
        CHECK(std::abs(value) < 0.1f);
    }
}

TEST_CASE("FxEngine allpass filter", "[fxengine][allpass]") {
    TestEngine engine;
    float buffer[kTestBufferSize] = {};
    engine.Init(buffer);

    using DL = TestEngine::DelayLine<TestMemory::Line1, 0>;
    DL delay_line;

    SECTION("WriteAllPass preserves previous read") {
        // First write a value
        TestEngine::Context ctx1;
        engine.Start(&ctx1);
        ctx1.Load(0.7f);
        ctx1.Write(delay_line, 0, 1.0f);

        // Advance and read
        TestEngine::Context ctx2;
        engine.Start(&ctx2);
        ctx2.Load(0.0f);
        ctx2.Read(delay_line, 1, 0.5f);  // This sets previous_read

        // Now write allpass - it should add previous_read to accumulator
        ctx2.Load(0.3f);
        ctx2.WriteAllPass(delay_line, 0, 0.5f);

        float value = 0.0f;
        ctx2.Write(value);
        // After WriteAllPass: accumulator = 0.3 * 0.5 + previous_read
        // previous_read from the Read was 0.7
        CHECK(value == Approx(0.3f * 0.5f + 0.7f));
    }
}

TEST_CASE("FxEngine interpolated read", "[fxengine][interpolate]") {
    TestEngine engine;
    float buffer[kTestBufferSize] = {};
    engine.Init(buffer);

    using DL = TestEngine::DelayLine<TestMemory::Line1, 0>;
    DL delay_line;

    SECTION("Interpolation between samples") {
        // Write two distinct values
        TestEngine::Context ctx1;
        engine.Start(&ctx1);
        ctx1.Load(0.0f);
        ctx1.Write(delay_line, 0, 1.0f);

        TestEngine::Context ctx2;
        engine.Start(&ctx2);
        ctx2.Load(1.0f);
        ctx2.Write(delay_line, 0, 1.0f);

        // Read with interpolation at 0.5 offset (between the two samples)
        TestEngine::Context ctx3;
        engine.Start(&ctx3);
        ctx3.Load(0.0f);
        ctx3.Interpolate(delay_line, 1.5f, 1.0f);

        float value = 0.0f;
        ctx3.Write(value);
        // Should be interpolated between 0.0 and 1.0
        CHECK(value >= 0.0f);
        CHECK(value <= 1.0f);
    }
}

TEST_CASE("FxEngine LFO", "[fxengine][lfo]") {
    TestEngine engine;
    float buffer[kTestBufferSize] = {};
    engine.Init(buffer);

    SECTION("LFO frequency can be set") {
        // This should not throw
        engine.SetLFOFrequency(clouds::LFO_1, 0.01f);
        engine.SetLFOFrequency(clouds::LFO_2, 0.02f);

        // Process some samples to exercise LFO
        for (int i = 0; i < 100; ++i) {
            TestEngine::Context ctx;
            engine.Start(&ctx);
        }
        // No crash = success
        CHECK(true);
    }
}

TEST_CASE("FxEngine DataType compression", "[fxengine][datatype]") {
    SECTION("FORMAT_32_BIT is passthrough") {
        float val = 0.12345f;
        auto compressed = clouds::DataType<clouds::FORMAT_32_BIT>::Compress(val);
        auto decompressed = clouds::DataType<clouds::FORMAT_32_BIT>::Decompress(compressed);
        CHECK(decompressed == Approx(val));
    }

    SECTION("FORMAT_16_BIT compresses to 16-bit range") {
        float val = 0.5f;
        auto compressed = clouds::DataType<clouds::FORMAT_16_BIT>::Compress(val);
        CHECK(compressed == static_cast<uint16_t>(16384));

        auto decompressed = clouds::DataType<clouds::FORMAT_16_BIT>::Decompress(compressed);
        CHECK(decompressed == Approx(0.5f).margin(0.001f));
    }

    SECTION("FORMAT_12_BIT compresses to 12-bit range") {
        float val = 0.5f;
        auto compressed = clouds::DataType<clouds::FORMAT_12_BIT>::Compress(val);
        CHECK(compressed == static_cast<uint16_t>(2048));

        auto decompressed = clouds::DataType<clouds::FORMAT_12_BIT>::Decompress(compressed);
        CHECK(decompressed == Approx(0.5f).margin(0.001f));
    }

    SECTION("FORMAT_16_BIT clips overflow") {
        float val = 2.0f;  // Beyond range
        auto compressed = clouds::DataType<clouds::FORMAT_16_BIT>::Compress(val);
        auto decompressed = clouds::DataType<clouds::FORMAT_16_BIT>::Decompress(compressed);
        CHECK(decompressed == Approx(1.0f).margin(0.001f));
    }
}

TEST_CASE("FxEngine multiple delay lines", "[fxengine][delay]") {
    TestEngine engine;
    float buffer[kTestBufferSize] = {};
    engine.Init(buffer);

    // DelayLine<Memory, index> where Memory is the tail and index counts back
    // Line1 is index 0 from Line3 (the deepest), Line2 is index 1, Line3 is index 2
    using Memory = TestMemory::Line3;  // Reserve<16, Reserve<32, Reserve<64>>>
    using DL1 = TestEngine::DelayLine<Memory, 2>;  // First declared (64 samples)
    using DL2 = TestEngine::DelayLine<Memory, 1>;  // Second declared (32 samples)
    using DL3 = TestEngine::DelayLine<Memory, 0>;  // Third declared (16 samples)
    DL1 delay1;
    DL2 delay2;
    DL3 delay3;

    SECTION("Delay lines have correct layout") {
        // DelayLine recursively navigates TailType - index 0 is the innermost (last declared)
        CHECK(DL1::length == 64);
        CHECK(DL2::length == 32);
        CHECK(DL3::length == 16);
        // Index 0 (DL3) starts at base 0, subsequent lines add length+1
        CHECK(DL3::base == 0);
        CHECK(DL2::base == 17);   // 0 + 16 + 1
        CHECK(DL1::base == 50);   // 17 + 32 + 1
    }

    SECTION("Independent delay lines") {
        // Write different values to each line
        TestEngine::Context ctx1;
        engine.Start(&ctx1);
        ctx1.Load(0.3f);
        ctx1.Write(delay1, 0, 1.0f);

        ctx1.Load(0.7f);
        ctx1.Write(delay2, 0, 1.0f);

        ctx1.Load(0.9f);
        ctx1.Write(delay3, 0, 1.0f);

        // Read back and verify independence
        TestEngine::Context ctx2;
        engine.Start(&ctx2);

        ctx2.Load(0.0f);
        ctx2.Read(delay1, 1, 1.0f);
        float val1 = 0.0f;
        ctx2.Write(val1);

        ctx2.Load(0.0f);
        ctx2.Read(delay2, 1, 1.0f);
        float val2 = 0.0f;
        ctx2.Write(val2);

        ctx2.Load(0.0f);
        ctx2.Read(delay3, 1, 1.0f);
        float val3 = 0.0f;
        ctx2.Write(val3);

        CHECK(val1 == Approx(0.3f));
        CHECK(val2 == Approx(0.7f));
        CHECK(val3 == Approx(0.9f));
    }
}
