/**
 * @file LorenzAttractorTests.cpp
 * @brief Unit tests for Lorenz attractor implementation
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "../src/audio/LorenzAttractor.h"
#include <vector>
#include <cmath>

using namespace GranularPlunderphonics;

// Helper function to calculate variance
float calculateVariance(const std::vector<float>& data) {
    float sum = 0.0f;
    float sumSq = 0.0f;

    for (float value : data) {
        sum += value;
        sumSq += value * value;
    }

    float mean = sum / data.size();
    return (sumSq / data.size()) - (mean * mean);
}

TEST_CASE("Lorenz System Basic Tests", "[lorenz]") {
    const double sampleRate = 44100.0;
    LorenzAttractor attractor(sampleRate);

    SECTION("Standard Parameters") {
        // Check default parameters
        auto params = attractor.getParameters();
        REQUIRE(params.rho == Catch::Approx(28.0));
        REQUIRE(params.beta == Catch::Approx(8.0/3.0));
        REQUIRE(params.sigma == Catch::Approx(10.0));

        // Process some samples and verify output range
        std::vector<float> buffer(1000);
        attractor.process(buffer.data(), buffer.size());

        // Check output bounds
        for (float sample : buffer) {
            REQUIRE(sample >= -1.0f);
            REQUIRE(sample <= 1.0f);
        }
    }

    SECTION("Parameter Changes") {
        // Test parameter modifications
        LorenzAttractor::Parameters params{
            .rho = 35.0,
            .beta = 3.0,
            .sigma = 12.0
        };
        attractor.setParameters(params);

        // Process and verify different behavior
        std::vector<float> buffer(1000);
        attractor.process(buffer.data(), buffer.size());

        // Calculate statistical properties
        float sum = 0.0f;
        float maxVal = -1.0f;
        float minVal = 1.0f;
        for (float sample : buffer) {
            sum += sample;
            maxVal = std::max(maxVal, sample);
            minVal = std::min(minVal, sample);
        }
        float mean = sum / buffer.size();

        // Verify statistics are different from standard parameters
        REQUIRE(mean != Catch::Approx(0.0f).margin(0.1f));
        REQUIRE(maxVal != minVal);
    }

    SECTION("Update Rate Control") {
        // Test different update rates
        std::vector<double> rates = {1.0, 10.0, 100.0, 1000.0};

        for (double rate : rates) {
            attractor.setUpdateRate(rate);
            std::vector<float> buffer(1000);
            attractor.process(buffer.data(), buffer.size());

            // Verify no discontinuities
            for (size_t i = 1; i < buffer.size(); ++i) {
                float diff = std::abs(buffer[i] - buffer[i-1]);
                REQUIRE(diff < 0.1f); // Maximum allowed step
            }
        }
    }

    SECTION("State Reset") {
        // Get initial output
        float firstSample = attractor.process();

        // Process some samples
        std::vector<float> buffer(1000);
        attractor.process(buffer.data(), buffer.size());

        // Reset state
        attractor.resetState();

        // Verify we get the same initial output
        float newFirstSample = attractor.process();
        REQUIRE(firstSample == Catch::Approx(newFirstSample));
    }

    SECTION("Edge Case Parameters") {
        // Test stable case
        LorenzAttractor::Parameters stableParams{
            .rho = 0.5,
            .beta = 2.0,
            .sigma = 4.0
        };
        attractor.setParameters(stableParams);

        std::vector<float> stableBuffer(1000);
        attractor.process(stableBuffer.data(), stableBuffer.size());

        // Test chaotic case
        LorenzAttractor::Parameters chaoticParams{
            .rho = 99.0,
            .beta = 8.0/3.0,
            .sigma = 10.0
        };
        attractor.setParameters(chaoticParams);

        std::vector<float> chaoticBuffer(1000);
        attractor.process(chaoticBuffer.data(), chaoticBuffer.size());

        // Compare variance between stable and chaotic
        float stableVar = calculateVariance(stableBuffer);
        float chaoticVar = calculateVariance(chaoticBuffer);

        REQUIRE(chaoticVar > stableVar);
    }
}