// test/GrainCloudTests.cpp

#include <catch2/catch_test_macros.hpp>
#include "../src/audio/GrainCloud.h"
#include "../src/audio/AudioBuffer.h"
#include <vector>
#include <cmath>

using namespace GranularPlunderphonics;

TEST_CASE("GrainCloud Basic Operation", "[graincloud]") {
    GrainCloud cloud(100);
    AudioBuffer source(1, 44100); // 1 second mono source
    AudioBuffer output(2, 1024);  // Stereo output buffer

    // Create temporary buffer for sine wave
    std::vector<float> sineWave(44100);
    for (size_t i = 0; i < sineWave.size(); ++i) {
        sineWave[i] = std::sin(2.0f * 3.14159f * 440.0f * i / 44100.0f);
    }

    // Write sine wave to source buffer
    source.write(0, sineWave.data(), sineWave.size(), 0);

    SECTION("Low Density") {
        cloud.setDensity(1.0f); // 1 grain per second
        cloud.process(source, output, 1024);

        // Verify output has some non-zero content
        bool hasContent = false;
        for (size_t i = 0; i < 1024; ++i) {
            if (std::abs(output.getSample(0, i)) > 0.0f) {
                hasContent = true;
                break;
            }
        }
        REQUIRE(hasContent);
    }

    SECTION("High Density") {
        cloud.setDensity(100.0f); // 100 grains per second
        RandomizationParams params;
        params.positionVariation = 0.5f;
        params.sizeVariation = 0.5f;
        cloud.setRandomization(params);

        cloud.process(source, output, 1024);

        // Verify output has significant content
        float maxAmplitude = 0.0f;
        for (size_t i = 0; i < 1024; ++i) {
            maxAmplitude = std::max(maxAmplitude,
                std::abs(output.getSample(0, i)));
        }
        REQUIRE(maxAmplitude > 0.1f);
    }

    SECTION("Randomization") {
        cloud.setDensity(10.0f);
        RandomizationParams params;
        params.positionVariation = 1.0f;
        cloud.setRandomization(params);

        AudioBuffer output1(2, 1024);
        AudioBuffer output2(2, 1024);

        cloud.process(source, output1, 1024);
        cloud.process(source, output2, 1024);

        // Verify outputs are different (randomization working)
        bool isDifferent = false;
        for (size_t i = 0; i < 1024; ++i) {
            if (output1.getSample(0, i) != output2.getSample(0, i)) {
                isDifferent = true;
                break;
            }
        }
        REQUIRE(isDifferent);
    }
}
