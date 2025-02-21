/**
 * @file GrainEngine.cpp
 * @brief Implementation of the granular synthesis engine
 */

#include "GrainEngine.h"
#include <cmath>

namespace GranularPlunderphonics {

GrainEngine::GrainEngine(double sampleRate, size_t maxGrains)
    : mSampleRate(sampleRate)
    , mMaxGrains(maxGrains)
    , mGrainSizeMs(50.0f)
    , mGrainDensity(10.0f)
    , mGrainShape(GrainShapeType::Gaussian)
    , mNextGrainTime(0)
    , mLogger("GrainEngine")
{
    mLogger.info("Creating GrainEngine instance");

    // Initialize grain pool
    mGrains.resize(mMaxGrains);

    // Fill free grain indices queue
    for (size_t i = 0; i < mMaxGrains; ++i) {
        mFreeGrainIndices.push(i);
    }
}

bool GrainEngine::process(const AudioBuffer& input, AudioBuffer& output, size_t numSamples) {
    if (input.getNumChannels() < 1 || output.getNumChannels() < 2) {
        mLogger.error("Invalid buffer configuration");
        return false;
    }

    // Clear output buffer
    output.clear();

    // Calculate samples between grains based on density
    size_t samplesPerGrain = static_cast<size_t>(mSampleRate / mGrainDensity.load());

    // Process each sample
    for (size_t i = 0; i < numSamples; ++i) {
        // Check if it's time to trigger a new grain
        if (mNextGrainTime.load() <= 0) {
            // Calculate random position in source audio
            size_t sourcePos = rand() % input.getNumSamples();
            triggerGrain(sourcePos);

            // Reset next grain time
            mNextGrainTime.store(samplesPerGrain);
        }

        // Update and mix all active grains
        updateGrains(1);

        // Mix grain outputs into main output
        std::lock_guard<std::mutex> lock(mGrainMutex);
        for (const auto& grain : mGrains) {
            if (grain.active) {
                float grainSample = input.getSample(0, grain.currentPosition) *
                                  calculateEnvelope(grain, grain.currentPosition - grain.startPosition);

                // Apply to both channels with slight stereo spread
                output.addSample(0, i, grainSample * 1.0f);
                output.addSample(1, i, grainSample * 0.9f);
            }
        }

        // Decrement next grain time
        mNextGrainTime.fetch_sub(1);
    }

    return true;
}

void GrainEngine::setGrainSize(float sizeMs) {
    mGrainSizeMs.store(std::max(1.0f, std::min(1000.0f, sizeMs)));
}

void GrainEngine::setGrainDensity(float grainsPerSecond) {
    mGrainDensity.store(std::max(0.1f, std::min(100.0f, grainsPerSecond)));
}

void GrainEngine::setGrainShape(GrainShapeType shape) {
    mGrainShape.store(shape);
}

void GrainEngine::reset() {
    std::lock_guard<std::mutex> lock(mGrainMutex);

    // Clear all active grains
    for (auto& grain : mGrains) {
        grain.active = false;
    }

    // Reset free grain queue
    while (!mFreeGrainIndices.empty()) {
        mFreeGrainIndices.pop();
    }

    // Refill free grain queue
    for (size_t i = 0; i < mMaxGrains; ++i) {
        mFreeGrainIndices.push(i);
    }

    mNextGrainTime.store(0);
}

bool GrainEngine::triggerGrain(size_t sourcePos) {
    std::lock_guard<std::mutex> lock(mGrainMutex);

    if (mFreeGrainIndices.empty()) {
        return false;
    }

    // Get next free grain index
    size_t grainIndex = mFreeGrainIndices.front();
    mFreeGrainIndices.pop();

    // Calculate grain size in samples
    size_t grainSizeSamples = static_cast<size_t>(mGrainSizeMs.load() * mSampleRate / 1000.0);

    // Initialize grain
    Grain& grain = mGrains[grainIndex];
    grain.startPosition = sourcePos;
    grain.currentPosition = sourcePos;
    grain.grainSize = grainSizeSamples;
    grain.amplitude = 1.0f;
    grain.shape = mGrainShape.load();
    grain.active = true;

    return true;
}

float GrainEngine::calculateEnvelope(const Grain& grain, size_t position) const {
    if (position >= grain.grainSize) {
        return 0.0f;
    }

    float normalizedPos = static_cast<float>(position) / grain.grainSize;

    switch (grain.shape) {
        case GrainShapeType::Sine:
            return std::sin(M_PI * normalizedPos);

        case GrainShapeType::Triangle:
            return 1.0f - std::abs(2.0f * normalizedPos - 1.0f);

        case GrainShapeType::Rectangle:
            return 1.0f;

        case GrainShapeType::Gaussian:
        default: {
            float x = (normalizedPos - 0.5f) * 6.0f; // Scale for -3 to 3 range
            return std::exp(-x * x / 2.0f);
        }
    }
}

void GrainEngine::updateGrains(size_t numSamples) {
    std::lock_guard<std::mutex> lock(mGrainMutex);

    for (auto& grain : mGrains) {
        if (!grain.active) {
            continue;
        }

        // Update position
        grain.currentPosition += numSamples;

        // Check if grain is finished
        if (grain.currentPosition >= grain.startPosition + grain.grainSize) {
            grain.active = false;
            mFreeGrainIndices.push(&grain - mGrains.data()); // Calculate index from pointer
        }
    }
}

} // namespace GranularPlunderphonics
