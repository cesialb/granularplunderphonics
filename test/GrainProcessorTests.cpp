#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "../src/audio/GrainProcessor.h"
#include <cmath>

using namespace GranularPlunderphonics;

namespace {
    // Helper to create test audio buffer with sine wave
    AudioBuffer createTestTone(double frequency, double sampleRate, size_t numSamples) {
        AudioBuffer buffer(1, numSamples);
        std::vector<float> data(numSamples);

        for (size_t i = 0; i < numSamples; ++i) {
            float t = static_cast<float>(i) / sampleRate;
            data[i] = std::sin(2.0 * M_PI * frequency * t);
        }

        buffer.write(0, data.data(), data.size(), 0);
        return buffer;
    }

    // Helper to detect fundamental frequency
    float detectFrequency(const AudioBuffer& buffer) {
        std::vector<float> data(buffer.getNumSamples());
        buffer.read(0, data.data(), data.size(), 0);

        // Count zero crossings
        int crossings = 0;
        for (size_t i = 1; i < data.size(); ++i) {
            if ((data[i - 1] >= 0 && data[i] < 0) ||
                (data[i - 1] < 0 && data[i] >= 0)) {
                crossings++;
            }
        }

        // Calculate frequency from zero crossings
        float duration = static_cast<float>(data.size()) / 44100.0f; // Assuming 44.1kHz
        return (crossings / 2.0f) / duration;
    }

    // Helper to calculate RMS amplitude
    float calculateRMS(const AudioBuffer& buffer, size_t channel) {
        std::vector<float> data(buffer.getNumSamples());
        buffer.read(channel, data.data(), data.size(), 0);

        float sum = 0.0f;
        for (float sample : data) {
            sum += sample * sample;
        }
        return std::sqrt(sum / data.size());
    }
} // namespace

TEST_CASE("Pitch Shifting Tests", "[grainprocessor]") {
    const double sampleRate = 44100.0;
    GrainProcessor processor(sampleRate);

    SECTION("Octave Shifts") {
        // Create test grain with 440Hz sine wave
        AudioBuffer grain = createTestTone(440.0, sampleRate, 2048);

        std::vector<float> pitchFactors = {0.5f, 2.0f}; // Octave down/up
        for (float factor : pitchFactors) {
            GrainProcessingConfig config;
            config.pitchShift = factor;

            AudioBuffer processed = grain;
            processor.processGrain(processed, config);

            // Verify frequency content
            float expectedFreq = 440.0f * factor;
            REQUIRE(detectFrequency(processed) == Catch::Approx(expectedFreq).margin(1.0));
        }
    }

    SECTION("Time Stretching") {
        AudioBuffer grain = createTestTone(440.0, sampleRate, 2048);

        std::vector<float> stretchFactors = {0.5f, 2.0f};
        for (float factor : stretchFactors) {
            GrainProcessingConfig config;
            config.timeStretch = factor;

            AudioBuffer processed = grain;
            processor.processGrain(processed, config);

            // Verify duration and pitch preservation
            REQUIRE(processed.getNumSamples() ==
                static_cast<size_t>(grain.getNumSamples() * factor));
            REQUIRE(detectFrequency(processed) == Catch::Approx(440.0).margin(1.0));
        }
    }

    SECTION("Stereo Positioning") {
        AudioBuffer grain = createTestTone(440.0, sampleRate, 2048);

        std::vector<float> positions = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
        for (float pos : positions) {
            GrainProcessingConfig config;
            config.stereoPosition = pos;

            AudioBuffer processed = grain;
            processor.processGrain(processed, config);

            // Verify channel balance
            float leftRMS = calculateRMS(processed, 0);
            float rightRMS = calculateRMS(processed, 1);

            if (pos < 0.5f) {
                REQUIRE(leftRMS > rightRMS);
            } else if (pos > 0.5f) {
                REQUIRE(rightRMS > leftRMS);
            } else {
                REQUIRE(leftRMS == Catch::Approx(rightRMS).margin(0.01));
            }
        }
    }
}
