// GrainCloud.cpp
#include "GrainCloud.h"
#include <algorithm>
#include <cmath>

namespace GranularPlunderphonics {

GrainCloud::GrainCloud(size_t maxGrains)
    : mMaxGrains(maxGrains)
    , mRNG(std::random_device{}()) {
    mGrains.resize(maxGrains);
}

void GrainCloud::setDensity(float grainsPerSecond) {
    mDensity = std::clamp(grainsPerSecond, 0.1f, 100.0f);
    mSamplesPerGrain = static_cast<size_t>(44100.0f / mDensity); // Assuming 44.1kHz
}

void GrainCloud::setRandomization(const RandomizationParams& params) {
    mRandomParams = params;
}

void GrainCloud::process(const AudioBuffer& source, AudioBuffer& output, size_t numSamples) {
    static const float TWO_PI = 6.28318530718f;

    // Schedule new grains based on density
    static size_t sampleCounter = 0;
    sampleCounter += numSamples;

    if (sampleCounter >= mSamplesPerGrain) {
        scheduleNewGrain();
        sampleCounter = 0;
    }

    // Process active grains
    for (auto& grain : mGrains) {
        if (!grain.active) continue;

        for (size_t i = 0; i < numSamples; ++i) {
            if (grain.currentSample >= grain.size * 44100) {
                grain.active = false;
                break;
            }

            // Calculate grain envelope
            float phase = static_cast<float>(grain.currentSample) / (grain.size * 44100);
            float envelope = 0.5f * (1.0f - std::cos(TWO_PI * phase));

            // Apply crossfading
            float crossfade = calculateCrossfade(grain);

            // Read from source with position variation
            float sourcePos = grain.position + mRandomParams.positionVariation *
                            (mUniformDist(mRNG) - 0.5f);
            sourcePos = std::clamp(sourcePos, 0.0f, 1.0f);

            // Mix into output
            for (size_t ch = 0; ch < output.getNumChannels(); ++ch) {
                float sample = source.getSample(ch,
                    static_cast<size_t>(sourcePos * source.getSize()));
                output.addSample(ch, i,
                    sample * envelope * crossfade * grain.amplitude);
            }

            grain.currentSample++;
        }
    }
}

void GrainCloud::scheduleNewGrain() {
    // Find inactive grain or oldest active grain
    size_t grainIndex = 0;
    size_t oldestGrain = 0;
    size_t maxAge = 0;

    for (size_t i = 0; i < mGrains.size(); ++i) {
        if (!mGrains[i].active) {
            grainIndex = i;
            break;
        }
        if (mGrains[i].currentSample > maxAge) {
            maxAge = mGrains[i].currentSample;
            oldestGrain = i;
        }
    }

    if (mGrains[grainIndex].active) {
        grainIndex = oldestGrain;
    }

    // Initialize new grain
    auto& grain = mGrains[grainIndex];
    grain.active = true;
    grain.position = mUniformDist(mRNG);
    grain.size = 0.05f * (1.0f + mRandomParams.sizeVariation *
                 (mUniformDist(mRNG) - 0.5f));
    grain.amplitude = 1.0f * (1.0f + mRandomParams.amplitudeVariation *
                    (mUniformDist(mRNG) - 0.5f));
    grain.shape = mRandomParams.shapeVariation * mUniformDist(mRNG);
    grain.currentSample = 0;
}

float GrainCloud::calculateCrossfade(const GrainParameters& grain) const {
    static const float CROSSFADE_TIME = 0.005f; // 5ms crossfade
    float fadeTime = CROSSFADE_TIME * 44100; // samples

    if (grain.currentSample < fadeTime) {
        return grain.currentSample / fadeTime;
    }
    else if (grain.currentSample > (grain.size * 44100 - fadeTime)) {
        return (grain.size * 44100 - grain.currentSample) / fadeTime;
    }
    return 1.0f;
}

} // namespace GranularPlunderphonics
