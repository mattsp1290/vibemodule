#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <clouds/fx_engine.h>
#include <cmath>

using Catch::Approx;

// Test engine configuration
constexpr size_t kTestBufferSize = 1024;
using TestEngine = clouds::FxEngine<kTestBufferSize, clouds::FORMAT_32_BIT>;

// Memory layout for allpass testing
struct AllpassMemory {
    using Line1 = TestEngine::Reserve<256>;
};

TEST_CASE("Allpass WriteAllPass adds previous_read to output", "[allpass]") {
    TestEngine engine;
    float buffer[kTestBufferSize] = {};
    engine.Init(buffer);

    using DL = TestEngine::DelayLine<AllpassMemory::Line1, 0>;
    DL delay_line;

    SECTION("WriteAllPass incorporates previous_read") {
        // First, write a known value to the delay line
        TestEngine::Context ctx1;
        engine.Start(&ctx1);
        ctx1.Load(0.5f);
        ctx1.Write(delay_line, 0, 1.0f);

        // Now read from the delay line (which sets previous_read)
        TestEngine::Context ctx2;
        engine.Start(&ctx2);
        ctx2.Load(0.0f);
        ctx2.Read(delay_line, 1, 1.0f);  // This should read 0.5 and set previous_read

        // Store what was read
        float readValue = 0.0f;
        ctx2.Write(readValue);
        CHECK(readValue == Approx(0.5f));

        // Now load a new value and WriteAllPass
        ctx2.Load(0.3f);
        ctx2.WriteAllPass(delay_line, 0, 0.7f);

        float afterAllpass = 0.0f;
        ctx2.Write(afterAllpass);

        // After WriteAllPass: accumulator = 0.3 * 0.7 + previous_read(0.5)
        CHECK(afterAllpass == Approx(0.3f * 0.7f + 0.5f));
    }
}

TEST_CASE("Allpass filter pattern produces dispersed output", "[allpass]") {
    TestEngine engine;
    float buffer[kTestBufferSize] = {};
    engine.Init(buffer);

    using DL = TestEngine::DelayLine<AllpassMemory::Line1, 0>;
    DL delay_line;

    constexpr float kap = 0.5f;  // Allpass coefficient

    SECTION("Impulse response shows characteristic reflections") {
        std::vector<float> impulseResponse;

        for (int i = 0; i < 300; ++i) {
            float input = (i == 0) ? 1.0f : 0.0f;

            TestEngine::Context c;
            engine.Start(&c);

            // Standard allpass pattern from Reverb class:
            // c.Read(ap TAIL, kap);  // Read from end of delay
            // c.WriteAllPass(ap, -kap);  // Write to beginning

            c.Load(input);  // Start with input
            c.Read(delay_line, static_cast<int32_t>(DL::length - 1), kap);  // Add delayed * kap
            c.WriteAllPass(delay_line, -kap);  // Write with -kap, add previous_read

            float output = 0.0f;
            c.Write(output);
            impulseResponse.push_back(output);
        }

        // The impulse response should have:
        // 1. An initial output (at sample 0)
        // 2. A reflection after the delay length
        CHECK(std::abs(impulseResponse[0]) > 0.1f);  // Initial response

        // Find significant samples after delay
        int significantSamples = 0;
        for (size_t i = DL::length - 10; i < std::min(impulseResponse.size(), static_cast<size_t>(DL::length + 10)); ++i) {
            if (std::abs(impulseResponse[i]) > 0.01f) {
                significantSamples++;
            }
        }
        // Should have some activity around the delay length
        CHECK(significantSamples > 0);
    }
}

TEST_CASE("Allpass coefficient affects reflection amplitude", "[allpass]") {
    float buffer[kTestBufferSize] = {};

    using DL = TestEngine::DelayLine<AllpassMemory::Line1, 0>;
    DL delay_line;

    SECTION("Different coefficients produce different responses") {
        std::vector<float> lowCoeffResponse;
        std::vector<float> highCoeffResponse;

        // Test with low coefficient (0.3)
        {
            TestEngine engine;
            engine.Init(buffer);
            constexpr float kap = 0.3f;

            for (int i = 0; i < 300; ++i) {
                float input = (i == 0) ? 1.0f : 0.0f;

                TestEngine::Context c;
                engine.Start(&c);
                c.Load(input);
                c.Read(delay_line, static_cast<int32_t>(DL::length - 1), kap);
                c.WriteAllPass(delay_line, -kap);

                float output = 0.0f;
                c.Write(output);
                lowCoeffResponse.push_back(output);
            }
        }

        // Test with high coefficient (0.7)
        {
            TestEngine engine;
            engine.Init(buffer);
            constexpr float kap = 0.7f;

            for (int i = 0; i < 300; ++i) {
                float input = (i == 0) ? 1.0f : 0.0f;

                TestEngine::Context c;
                engine.Start(&c);
                c.Load(input);
                c.Read(delay_line, static_cast<int32_t>(DL::length - 1), kap);
                c.WriteAllPass(delay_line, -kap);

                float output = 0.0f;
                c.Write(output);
                highCoeffResponse.push_back(output);
            }
        }

        // The initial response amplitude should differ based on coefficient
        // With higher coefficient, less goes directly through, more is fed back
        CHECK(std::abs(lowCoeffResponse[0]) != Approx(std::abs(highCoeffResponse[0])).margin(0.1f));
    }
}

TEST_CASE("Cascaded allpass filters increase dispersion", "[allpass]") {
    TestEngine engine;
    float buffer[kTestBufferSize] = {};
    engine.Init(buffer);

    // Use multiple delay lines
    struct CascadeMemory {
        using Line1 = TestEngine::Reserve<113>;
        using Line2 = TestEngine::Reserve<162, Line1>;
        using Line3 = TestEngine::Reserve<241, Line2>;
        using Line4 = TestEngine::Reserve<399, Line3>;
    };

    using AP1 = TestEngine::DelayLine<CascadeMemory::Line4, 3>;
    using AP2 = TestEngine::DelayLine<CascadeMemory::Line4, 2>;
    using AP3 = TestEngine::DelayLine<CascadeMemory::Line4, 1>;
    using AP4 = TestEngine::DelayLine<CascadeMemory::Line4, 0>;
    AP1 ap1;
    AP2 ap2;
    AP3 ap3;
    AP4 ap4;

    constexpr float kap = 0.625f;  // Typical diffusion coefficient

    SECTION("Four allpass cascade spreads impulse") {
        std::vector<float> impulseResponse;

        for (int i = 0; i < 1000; ++i) {
            float input = (i == 0) ? 1.0f : 0.0f;

            TestEngine::Context c;
            engine.Start(&c);

            // 4 cascaded allpass filters (as in Reverb input section)
            c.Load(input);
            c.Read(ap1, static_cast<int32_t>(AP1::length - 1), kap);
            c.WriteAllPass(ap1, -kap);

            float temp = 0.0f;
            c.Write(temp);
            c.Load(temp);
            c.Read(ap2, static_cast<int32_t>(AP2::length - 1), kap);
            c.WriteAllPass(ap2, -kap);

            c.Write(temp);
            c.Load(temp);
            c.Read(ap3, static_cast<int32_t>(AP3::length - 1), kap);
            c.WriteAllPass(ap3, -kap);

            c.Write(temp);
            c.Load(temp);
            c.Read(ap4, static_cast<int32_t>(AP4::length - 1), kap);
            c.WriteAllPass(ap4, -kap);

            float output = 0.0f;
            c.Write(output);
            impulseResponse.push_back(output);
        }

        // Calculate where significant energy exists
        float totalEnergy = 0.0f;
        for (float s : impulseResponse) {
            totalEnergy += s * s;
        }

        // Find the time span where 90% of energy is contained
        float cumulativeEnergy = 0.0f;
        int start10 = 0;
        int end90 = 0;
        for (size_t i = 0; i < impulseResponse.size(); ++i) {
            cumulativeEnergy += impulseResponse[i] * impulseResponse[i];
            if (cumulativeEnergy >= totalEnergy * 0.05f && start10 == 0) {
                start10 = static_cast<int>(i);
            }
            if (cumulativeEnergy >= totalEnergy * 0.95f && end90 == 0) {
                end90 = static_cast<int>(i);
            }
        }

        // The impulse should be spread over many samples (dispersion)
        int spreadDuration = end90 - start10;
        CHECK(spreadDuration > 50);  // Significant spreading

        // Energy should extend beyond the longest single delay
        CHECK(end90 > static_cast<int>(AP4::length));
    }
}

TEST_CASE("Allpass preserves total signal energy over time", "[allpass]") {
    TestEngine engine;
    float buffer[kTestBufferSize] = {};
    engine.Init(buffer);

    using DL = TestEngine::DelayLine<AllpassMemory::Line1, 0>;
    DL delay_line;

    constexpr float kap = 0.5f;

    SECTION("Total energy over long impulse response approaches input energy") {
        float inputEnergy = 1.0f;  // Single impulse
        float outputEnergy = 0.0f;

        // Process for long enough to capture all reflections
        for (int i = 0; i < 1000; ++i) {
            float input = (i == 0) ? 1.0f : 0.0f;

            TestEngine::Context c;
            engine.Start(&c);
            c.Load(input);
            c.Read(delay_line, static_cast<int32_t>(DL::length - 1), kap);
            c.WriteAllPass(delay_line, -kap);

            float output = 0.0f;
            c.Write(output);
            outputEnergy += output * output;
        }

        // Total output energy should be close to input energy
        // (allpass filters preserve energy in theory)
        // Allow some margin for the discrete implementation
        CHECK(outputEnergy > inputEnergy * 0.8f);
        CHECK(outputEnergy < inputEnergy * 1.2f);
    }
}
