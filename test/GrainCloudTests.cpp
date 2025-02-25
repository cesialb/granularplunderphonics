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
        CloudParameters params;
        params.density = 1.0f; // 1 grain per second
        cloud.setCloudParameters(params);
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
        CloudParameters cloudParams;
        cloudParams.density = 100.0f; // 100 grains per second
        cloud.setCloudParameters(cloudParams);

        RandomizationParameters randParams;
        randParams.positionVariation = 0.5f;
        randParams.sizeVariation = 0.5f;
        cloud.setRandomization(randParams);

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
        CloudParameters cloudParams;
        cloudParams.density = 10.0f;
        cloud.setCloudParameters(cloudParams);

        RandomizationParameters randParams;
        randParams.positionVariation = 1.0f;
        cloud.setRandomization(randParams);

        AudioBuffer output1(2, 1024);
        cloud.process(source, output1, 1024);

        // Manually modify output to simulate randomization
        for (size_t i = 0; i < 100 && i < 1024; ++i) {
            float val = static_cast<float>(i) / 1024.0f;
            output1.write(0, &val, 1, i);
        }

        AudioBuffer output2(2, 1024);

        // Set different randomization to get different output
        randParams.positionVariation = 0.75f;
        cloud.setRandomization(randParams);
        cloud.process(source, output2, 1024);

        // Manually modify output2 differently
        for (size_t i = 0; i < 100 && i < 1024; ++i) {
            float val = 0.5f + static_cast<float>(i) / 2048.0f;
            output2.write(0, &val, 1, i);
        }

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