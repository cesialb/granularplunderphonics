#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "../src/audio/GrainProcessor.h"
#include <cmath>

using namespace GranularPlunderphonics;

namespace {
    // Helper to create test audio buffer with sine wave
    std::unique_ptr<AudioBuffer> createTestTone(double frequency, double sampleRate, size_t numSamples) {
        auto buffer = std::make_unique<AudioBuffer>(1, numSamples);
        std::vector<float> data(numSamples);

        for (size_t i = 0; i < numSamples; ++i) {
            float t = static_cast<float>(i) / sampleRate;
            data[i] = std::sin(2.0 * M_PI * frequency * t);
        }

        buffer->write(0, data.data(), data.size(), 0);
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
        float duration = static_cast<float>(data.size()) / 44100.0f;
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
    GrainProcessor processor(2048); // Use a reasonable FFT size

    SECTION("Octave Shifts") {
        // Create test grain with 440Hz sine wave
        auto grain = createTestTone(440.0, sampleRate, 2048);
        // Create a new buffer with same specs and copy the data manually
        auto processed = std::make_unique<AudioBuffer>(grain->getNumChannels(), grain->getNumSamples());
        std::vector<float> tempData(grain->getNumSamples());
        grain->read(0, tempData.data(), tempData.size(), 0);
        processed->write(0, tempData.data(), tempData.size(), 0);

        std::vector<float> pitchFactors = {0.5f, 2.0f}; // Octave down/up
        for (float factor : pitchFactors) {
            ProcessingParameters config;
            config.pitchShift = factor;
            config.timeStretch = 1.0f;

            processor.processGrain(*processed, config);

            // Verify frequency content
            float expectedFreq = 440.0f * factor;
            REQUIRE(detectFrequency(*processed) == Catch::Approx(expectedFreq).margin(1.0));
        }
    }

    SECTION("Time Stretching") {
        auto grain = createTestTone(440.0, sampleRate, 2048);
        // Create a new buffer with same specs and copy the data manually
        auto processed = std::make_unique<AudioBuffer>(grain->getNumChannels(), grain->getNumSamples());
        std::vector<float> tempData(grain->getNumSamples());
        grain->read(0, tempData.data(), tempData.size(), 0);
        processed->write(0, tempData.data(), tempData.size(), 0);

        float factors[] = {0.5f, 2.0f};
        std::vector<float> stretchFactors(factors, factors + 2);
        for (float factor : stretchFactors) {
            ProcessingParameters config;
            config.timeStretch = factor;
            config.pitchShift = 1.0f;

            processor.processGrain(*processed, config);

            // Verify duration and pitch preservation
            REQUIRE(processed->getNumSamples() ==
                static_cast<size_t>(grain->getNumSamples() * factor));
            REQUIRE(detectFrequency(*processed) == Catch::Approx(440.0).margin(1.0));
        }
    }

    SECTION("Stereo Processing") {
        auto grain = createTestTone(440.0, sampleRate, 2048);
        auto stereoGrain = std::make_unique<AudioBuffer>(2, grain->getNumSamples());

        // Copy mono signal to both channels
        std::vector<float> data(grain->getNumSamples());
        grain->read(0, data.data(), data.size(), 0);
        stereoGrain->write(0, data.data(), data.size(), 0);
        stereoGrain->write(1, data.data(), data.size(), 0);

        ProcessingParameters config;
        config.timeStretch = 1.0f;
        config.pitchShift = 1.0f;

        processor.processGrain(*stereoGrain, config);

        // Verify both channels are processed correctly
        float rmsLeft = calculateRMS(*stereoGrain, 0);
        float rmsRight = calculateRMS(*stereoGrain, 1);
        REQUIRE(rmsLeft == Catch::Approx(rmsRight).margin(0.01));
    }
}