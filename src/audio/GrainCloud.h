// GrainCloud.h
#pragma once
#include <vector>
#include <random>
#include <memory>
#include "AudioBuffer.h"

namespace GranularPlunderphonics {

    struct RandomizationParams {
        float positionVariation{0.1f};    // 0.0-1.0, percentage of source length
        float sizeVariation{0.2f};        // 0.0-1.0, percentage of base grain size
        float amplitudeVariation{0.1f};   // 0.0-1.0, percentage of base amplitude
        float shapeVariation{0.1f};       // 0.0-1.0, affects envelope shape
    };

    struct GrainParameters {
        float position{0.0f};     // Position in source (0.0-1.0)
        float size{0.05f};        // Grain size in seconds
        float amplitude{1.0f};    // Amplitude scaling
        float shape{0.0f};        // Shape parameter for envelope
        bool active{false};       // Whether grain is currently playing
        size_t startSample{0};    // Start sample in output buffer
        size_t currentSample{0};  // Current playback position
    };

    class GrainCloud {
    public:
        GrainCloud(size_t maxGrains = 100);

        void setDensity(float grainPerSecond);
        void setRandomization(const RandomizationParams& params);
        void process(const AudioBuffer& source, AudioBuffer& output, size_t numSamples);

    private:
        void scheduleNewGrain();
        void updateActiveGrains();
        float calculateCrossfade(const GrainParameters& grain) const;

        std::vector<GrainParameters> mGrains;
        RandomizationParams mRandomParams;
        float mDensity{10.0f};
        size_t mSamplesPerGrain{0};
        size_t mMaxGrains;

        std::mt19937 mRNG;
        std::uniform_real_distribution<float> mUniformDist{0.0f, 1.0f};
    };

} // namespace GranularPlunderphonics
