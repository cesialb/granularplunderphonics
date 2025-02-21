/**
 * @file GrainEngine.cpp
 * @brief Enhanced implementation of the granular synthesis engine
 */

#include "GrainEngine.h"
#include <random>
#include <algorithm>
#include <cmath>

namespace GranularPlunderphonics {

GrainEngine::GrainEngine(double sampleRate, size_t maxGrains)
    : mSampleRate(sampleRate)
    , mMaxGrains(maxGrains)
    , mGrainSizeMs(50.0f)
    , mGrainDensity(10.0f)
    , mGrainShape(GrainShapeType::Gaussian)
    , mGenerator(std::random_device{}())
    , mPosDistribution(0.0f, 1.0f)
    , mNextGrainTime(0)
    , mLogger("GrainEngine")
{
    mLogger.info("Creating GrainEngine instance");
    mGrains.resize(mMaxGrains);
    initializeWindowFunctions();
}

void GrainEngine::initializeWindowFunctions() {
    // Pre-calculate window functions for common grain sizes
    std::vector<size_t> commonSizes = {
        static_cast<size_t>(0.001f * mSampleRate),  // 1ms
        static_cast<size_t>(0.005f * mSampleRate),  // 5ms
        static_cast<size_t>(0.010f * mSampleRate),  // 10ms
        static_cast<size_t>(0.020f * mSampleRate),  // 20ms
        static_cast<size_t>(0.050f * mSampleRate),  // 50ms
        static_cast<size_t>(0.100f * mSampleRate)   // 100ms
    };

    for (size_t size : commonSizes) {
        for (int shape = 0; shape < 4; ++shape) {
            auto shapeType = static_cast<GrainShapeType>(shape);
            WindowKey key{shapeType, size};
            mWindowCache[key] = calculateWindow(shapeType, size);
        }
    }
}

std::vector<float> GrainEngine::calculateWindow(GrainShapeType shape, size_t size) {
    std::vector<float> window(size);
    const double pi = 3.14159265358979323846;

    switch (shape) {
        case GrainShapeType::Sine: {
            for (size_t i = 0; i < size; ++i) {
                double phase = (static_cast<double>(i) / (size - 1)) * pi;
                window[i] = static_cast<float>(std::sin(phase));
            }
            break;
        }

        case GrainShapeType::Triangle: {
            float increment = 1.0f / (size / 2);
            for (size_t i = 0; i < size / 2; ++i) {
                window[i] = i * increment;
                window[size - 1 - i] = window[i];
            }
            break;
        }

        case GrainShapeType::Rectangle: {
            std::fill(window.begin(), window.end(), 1.0f);

            // Apply short fade-in/out to prevent clicks
            size_t fadeLength = std::min(size_t(100), size / 10);
            for (size_t i = 0; i < fadeLength; ++i) {
                float fade = static_cast<float>(i) / fadeLength;
                window[i] *= fade;
                window[size - 1 - i] *= fade;
            }
            break;
        }

        case GrainShapeType::Gaussian: {
            double sigma = 0.4;
            double center = (size - 1) / 2.0;
            for (size_t i = 0; i < size; ++i) {
                double x = (i - center) / (size * sigma);
                window[i] = static_cast<float>(std::exp(-0.5 * x * x));
            }
            break;
        }
    }

    return window;
}

bool GrainEngine::process(AudioBuffer& input, AudioBuffer& output, size_t numSamples) {
    if (!validateBuffers(input, output, numSamples)) {
        return false;
    }

    // Clear output buffer
    output.clear();

    // Calculate time between grains based on density
    float samplesPerGrain = mSampleRate / mGrainDensity;

    // Process each sample
    for (size_t i = 0; i < numSamples; ++i) {
        // Check if it's time to trigger a new grain
        if (--mNextGrainTime <= 0) {
            triggerGrain(input);
            mNextGrainTime = static_cast<int>(samplesPerGrain);
        }

        // Process active grains
        processActiveGrains(input, output, i);
    }

    return true;
}

void GrainEngine::processActiveGrains(const AudioBuffer& input, AudioBuffer& output, size_t sampleIndex) {
    std::lock_guard<std::mutex> lock(mGrainMutex);

    for (auto& grain : mGrains) {
        if (!grain.active) continue;

        // Get grain position and calculate envelope
        size_t grainPos = grain.currentPosition - grain.startPosition;
        float envelope = getGrainEnvelope(grain, grainPos);

        // Calculate grain sample with interpolation
        float sample = 0.0f;
        if (grain.reverse) {
            sample = interpolateSample(input, 0, grain.startPosition - grainPos);
        } else {
            sample = interpolateSample(input, 0, grain.startPosition + grainPos);
        }

        // Apply envelope and mix to output
        sample *= envelope * grain.amplitude;

        // Apply spatialization
        float leftGain = std::cos(grain.pan * M_PI * 0.5f);
        float rightGain = std::sin(grain.pan * M_PI * 0.5f);

        output.addSample(0, sampleIndex, sample * leftGain);
        output.addSample(1, sampleIndex, sample * rightGain);

        // Update grain position
        grain.currentPosition++;

        // Check if grain is finished
        if (grainPos >= grain.grainSize) {
            grain.active = false;
        }
    }
}

float GrainEngine::getGrainEnvelope(const Grain& grain, size_t position) {
    if (position >= grain.grainSize) return 0.0f;

    // Get cached window if available
    WindowKey key{grain.shape, grain.grainSize};
    auto it = mWindowCache.find(key);
    if (it != mWindowCache.end()) {
        return it->second[position];
    }

    // Calculate window on-the-fly if not cached
    auto window = calculateWindow(grain.shape, grain.grainSize);
    return window[position];
}

float GrainEngine::interpolateSample(const AudioBuffer& buffer, size_t channel, float position) {
    if (position < 0 || position >= buffer.getNumSamples() - 1) {
        return 0.0f;
    }

    size_t pos0 = static_cast<size_t>(position);
    size_t pos1 = pos0 + 1;
    float frac = position - pos0;

    float sample0 = buffer.getSample(channel, pos0);
    float sample1 = buffer.getSample(channel, pos1);

    // Cubic interpolation
    float c0, c1, c2, c3;
    if (pos0 > 0 && pos1 < buffer.getNumSamples() - 1) {
        float sample_1 = buffer.getSample(channel, pos0 - 1);
        float sample2 = buffer.getSample(channel, pos1 + 1);

        c0 = sample_1;
        c1 = sample0;
        c2 = sample1;
        c3 = sample2;

        float p = frac;
        float p2 = p * p;
        float p3 = p2 * p;

        return c1 + 0.5f * p * (c2 - c0 + p * (2.0f * c0 - 5.0f * c1 + 4.0f * c2 - c3 + p * (3.0f * (c1 - c2) + c3 - c0)));
    }

    // Fall back to linear interpolation at buffer edges
    return sample0 + frac * (sample1 - sample0);
}

void GrainEngine::triggerGrain(const AudioBuffer& input) {
    std::lock_guard<std::mutex> lock(mGrainMutex);

    // Find inactive grain
    auto grainIt = std::find_if(mGrains.begin(), mGrains.end(),
                               [](const Grain& g) { return !g.active; });

    if (grainIt == mGrains.end()) {
        return; // No inactive grains available
    }

    // Calculate grain parameters
    float grainSizeSamples = (mGrainSizeMs / 1000.0f) * mSampleRate;

    // Add random variations
    float sizeVariation = mSizeVariation * (mPosDistribution(mGenerator) - 0.5f);
    float positionVariation = mPositionVariation * mPosDistribution(mGenerator);
    float panVariation = mPanVariation * (mPosDistribution(mGenerator) - 0.5f);

    // Initialize grain
    grainIt->active = true;
    grainIt->startPosition = static_cast<size_t>(input.getNumSamples() * positionVariation);
    grainIt->currentPosition = grainIt->startPosition;
    grainIt->grainSize = static_cast<size_t>(grainSizeSamples * (1.0f + sizeVariation));
    grainIt->amplitude = 1.0f;
    grainIt->shape = mGrainShape;
    grainIt->pan = std::clamp(0.5f + panVariation, 0.0f, 1.0f);
    grainIt->reverse = (mPosDistribution(mGenerator) < mReverseProb);
}

bool GrainEngine::validateBuffers(const AudioBuffer& input, const AudioBuffer& output, size_t numSamples) {
    if (input.getNumChannels() < 1 || output.getNumChannels() < 2) {
        mLogger.error("Invalid buffer configuration");
        return false;
    }

    if (input.getNumSamples() < numSamples || output.getNumSamples() < numSamples) {
        mLogger.error("Buffer size too small");
        return false;
    }

    return true;
}

void GrainEngine::setGrainSize(float sizeMs) {
    mGrainSizeMs = std::clamp(sizeMs, 1.0f, 1000.0f);
}

void GrainEngine::setGrainDensity(float grainsPerSecond) {
    mGrainDensity = std::clamp(grainsPerSecond, 0.1f, 1000.0f);
}

void GrainEngine::setGrainShape(GrainShapeType shape) {
    mGrainShape = shape;
}

void GrainEngine::setRandomization(float sizeVar, float posVar, float panVar, float revProb) {
    mSizeVariation = std::clamp(sizeVar, 0.0f, 1.0f);
    mPositionVariation = std::clamp(posVar, 0.0f, 1.0f);
    mPanVariation = std::clamp(panVar, 0.0f, 1.0f);
    mReverseProb = std::clamp(revProb, 0.0f, 1.0f);
}

} // namespace GranularPlunderphonics