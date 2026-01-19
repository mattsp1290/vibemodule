#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <stmlib/dsp/dsp.h>
#include <cmath>

using Catch::Approx;

TEST_CASE("stmlib::Interpolate linear interpolation", "[stmlib][dsp]") {
    const float table[] = {0.0f, 1.0f, 2.0f, 3.0f};
    const float size = 4.0f;

    SECTION("Exact indices") {
        CHECK(stmlib::Interpolate(table, 0.0f, size) == Approx(0.0f));
        CHECK(stmlib::Interpolate(table, 0.25f, size) == Approx(1.0f));
        CHECK(stmlib::Interpolate(table, 0.5f, size) == Approx(2.0f));
    }

    SECTION("Interpolated indices") {
        // Index 0.5 in a size-4 table = position 2.0, between table[2] and table[3]
        CHECK(stmlib::Interpolate(table, 0.125f, size) == Approx(0.5f));
        CHECK(stmlib::Interpolate(table, 0.375f, size) == Approx(1.5f));
    }

    SECTION("Boundary clamping") {
        CHECK(stmlib::Interpolate(table, -0.1f, size) == Approx(0.0f));
        CHECK(stmlib::Interpolate(table, 1.1f, size) == Approx(3.0f).margin(0.01f));
    }
}

TEST_CASE("stmlib::InterpolateHermite cubic interpolation", "[stmlib][dsp]") {
    // Smooth curve for testing: sine-like values
    const float table[] = {0.0f, 0.5f, 1.0f, 0.5f, 0.0f, -0.5f, -1.0f, -0.5f, 0.0f};
    const float size = 9.0f;

    SECTION("Midpoint interpolation is smooth") {
        // Hermite should produce smooth results at quarter points
        float val = stmlib::InterpolateHermite(table, 0.25f, size);
        // At normalized index 0.25, position = 2.25
        // This is between table[2]=1.0 and table[3]=0.5
        CHECK(val < 1.0f);
        CHECK(val > 0.5f);
    }

    SECTION("Boundary clamping") {
        // Very small index should clamp to valid range
        float val = stmlib::InterpolateHermite(table, 0.05f, size);
        CHECK(std::isfinite(val));
    }
}

TEST_CASE("stmlib::InterpolateWrap cyclic interpolation", "[stmlib][dsp]") {
    const float table[] = {0.0f, 1.0f, 0.0f, -1.0f};
    const float size = 4.0f;

    SECTION("Normal range") {
        CHECK(stmlib::InterpolateWrap(table, 0.0f, size) == Approx(0.0f));
        CHECK(stmlib::InterpolateWrap(table, 0.25f, size) == Approx(1.0f));
        CHECK(stmlib::InterpolateWrap(table, 0.5f, size) == Approx(0.0f));
    }

    SECTION("Wrapping at boundaries") {
        // Index > 1 should wrap
        CHECK(stmlib::InterpolateWrap(table, 1.0f, size) == Approx(0.0f));
        CHECK(stmlib::InterpolateWrap(table, 1.25f, size) == Approx(1.0f));

        // Negative index should wrap
        CHECK(stmlib::InterpolateWrap(table, -0.25f, size) == Approx(-1.0f));
    }
}

TEST_CASE("stmlib::SmoothStep", "[stmlib][dsp]") {
    SECTION("Boundary values") {
        CHECK(stmlib::SmoothStep(0.0f) == Approx(0.0f));
        CHECK(stmlib::SmoothStep(1.0f) == Approx(1.0f));
    }

    SECTION("Midpoint") {
        CHECK(stmlib::SmoothStep(0.5f) == Approx(0.5f));
    }

    SECTION("S-curve properties") {
        // SmoothStep should be symmetric around 0.5
        CHECK(stmlib::SmoothStep(0.25f) == Approx(1.0f - stmlib::SmoothStep(0.75f)));

        // Derivative at endpoints should be 0 (flat)
        float epsilon = 0.001f;
        float derivAtZero = (stmlib::SmoothStep(epsilon) - stmlib::SmoothStep(0.0f)) / epsilon;
        float derivAtOne = (stmlib::SmoothStep(1.0f) - stmlib::SmoothStep(1.0f - epsilon)) / epsilon;
        CHECK(derivAtZero == Approx(0.0f).margin(0.01f));
        CHECK(derivAtOne == Approx(0.0f).margin(0.01f));
    }
}

TEST_CASE("stmlib::Crossfade", "[stmlib][dsp]") {
    SECTION("Boundary values") {
        CHECK(stmlib::Crossfade(1.0f, 2.0f, 0.0f) == Approx(1.0f));
        CHECK(stmlib::Crossfade(1.0f, 2.0f, 1.0f) == Approx(2.0f));
    }

    SECTION("Linear interpolation") {
        CHECK(stmlib::Crossfade(0.0f, 10.0f, 0.5f) == Approx(5.0f));
        CHECK(stmlib::Crossfade(-1.0f, 1.0f, 0.25f) == Approx(-0.5f));
    }
}

TEST_CASE("stmlib::SoftLimit", "[stmlib][dsp]") {
    SECTION("Small values pass through nearly unchanged") {
        CHECK(stmlib::SoftLimit(0.0f) == Approx(0.0f));
        CHECK(stmlib::SoftLimit(0.1f) == Approx(0.1f).margin(0.01f));
        CHECK(stmlib::SoftLimit(-0.1f) == Approx(-0.1f).margin(0.01f));
    }

    SECTION("Larger values are compressed") {
        float limited = stmlib::SoftLimit(2.0f);
        CHECK(limited < 2.0f);
        CHECK(limited > 0.0f);
    }

    SECTION("Symmetry") {
        CHECK(stmlib::SoftLimit(1.5f) == Approx(-stmlib::SoftLimit(-1.5f)));
    }
}

TEST_CASE("stmlib::SoftClip", "[stmlib][dsp]") {
    SECTION("Passthrough in soft region") {
        CHECK(stmlib::SoftClip(0.0f) == Approx(0.0f));
        CHECK(std::abs(stmlib::SoftClip(0.5f)) < 1.0f);
    }

    SECTION("Hard clipping at extremes") {
        CHECK(stmlib::SoftClip(10.0f) == 1.0f);
        CHECK(stmlib::SoftClip(-10.0f) == -1.0f);
        CHECK(stmlib::SoftClip(3.5f) == 1.0f);
        CHECK(stmlib::SoftClip(-3.5f) == -1.0f);
    }

    SECTION("Soft region boundary") {
        // Just inside the soft region
        CHECK(stmlib::SoftClip(2.9f) < 1.0f);
        CHECK(stmlib::SoftClip(-2.9f) > -1.0f);
    }
}

TEST_CASE("stmlib::Clip16", "[stmlib][dsp]") {
    SECTION("Values in range") {
        CHECK(stmlib::Clip16(0) == 0);
        CHECK(stmlib::Clip16(1000) == 1000);
        CHECK(stmlib::Clip16(-1000) == -1000);
        CHECK(stmlib::Clip16(32767) == 32767);
        CHECK(stmlib::Clip16(-32768) == -32768);
    }

    SECTION("Values out of range") {
        CHECK(stmlib::Clip16(40000) == 32767);
        CHECK(stmlib::Clip16(-40000) == -32768);
        CHECK(stmlib::Clip16(100000) == 32767);
        CHECK(stmlib::Clip16(-100000) == -32768);
    }
}

TEST_CASE("stmlib::ClipU16", "[stmlib][dsp]") {
    SECTION("Values in range") {
        CHECK(stmlib::ClipU16(0) == 0);
        CHECK(stmlib::ClipU16(1000) == 1000);
        CHECK(stmlib::ClipU16(65535) == 65535);
    }

    SECTION("Values out of range") {
        CHECK(stmlib::ClipU16(-100) == 0);
        CHECK(stmlib::ClipU16(70000) == 65535);
    }
}

TEST_CASE("stmlib::OnePoleCoefficient", "[stmlib][dsp]") {
    const float sampleRate = 48000.0f;

    SECTION("Low frequency gives small coefficient") {
        float coeff = stmlib::OnePoleCoefficient(100.0f, sampleRate);
        CHECK(coeff > 0.0f);
        CHECK(coeff < 0.1f);
    }

    SECTION("Higher frequency gives larger coefficient") {
        float lowCoeff = stmlib::OnePoleCoefficient(100.0f, sampleRate);
        float highCoeff = stmlib::OnePoleCoefficient(1000.0f, sampleRate);
        CHECK(highCoeff > lowCoeff);
    }

    SECTION("Coefficient approaches 1 at high frequencies") {
        float coeff = stmlib::OnePoleCoefficient(20000.0f, sampleRate);
        CHECK(coeff > 0.5f);
        CHECK(coeff <= 1.0f);
    }
}

TEST_CASE("stmlib ONE_POLE macro", "[stmlib][dsp]") {
    SECTION("Basic filtering") {
        float state = 0.0f;
        float coefficient = 0.5f;

        ONE_POLE(state, 1.0f, coefficient);
        CHECK(state == Approx(0.5f));

        ONE_POLE(state, 1.0f, coefficient);
        CHECK(state == Approx(0.75f));

        ONE_POLE(state, 1.0f, coefficient);
        CHECK(state == Approx(0.875f));
    }

    SECTION("Coefficient of 1 gives instant response") {
        float state = 0.0f;
        ONE_POLE(state, 5.0f, 1.0f);
        CHECK(state == Approx(5.0f));
    }

    SECTION("Coefficient of 0 gives no change") {
        float state = 2.0f;
        ONE_POLE(state, 10.0f, 0.0f);
        CHECK(state == Approx(2.0f));
    }
}

TEST_CASE("stmlib CONSTRAIN macro", "[stmlib][dsp]") {
    SECTION("Value in range unchanged") {
        float x = 0.5f;
        CONSTRAIN(x, 0.0f, 1.0f);
        CHECK(x == 0.5f);
    }

    SECTION("Value below min clamped") {
        float x = -0.5f;
        CONSTRAIN(x, 0.0f, 1.0f);
        CHECK(x == 0.0f);
    }

    SECTION("Value above max clamped") {
        float x = 1.5f;
        CONSTRAIN(x, 0.0f, 1.0f);
        CHECK(x == 1.0f);
    }
}
