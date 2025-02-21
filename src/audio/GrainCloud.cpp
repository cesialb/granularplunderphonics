/**
 * @file GrainCloud.cpp
 * @brief Implementation of enhanced grain cloud management system
 */

#include "GrainCloud.h"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace GranularPlunderphonics {

    GrainCloud::GrainCloud(size_t maxGrains, double sampleRate)
        : mMaxGrains(maxGrains)
        , mSampleRate(sampleRate)
        , mProcessor(std::make_unique<GrainProcessor>(2048)) // 2048 FFT size
        , mGenerator(std::random_device{}())
        , mDistribution(0.0f, 1.0f)
        , mLogger("GrainCloud")
    {
        mLogger.info("Creating GrainCloud instance");
        mGrains.resize(maxGrains);
        mOverlapCounts.resize(8192, 0.0f);
        reset();
    }

void GrainCloud::process(const AudioBuffer& source, AudioBuffer& output, size_t numSamples) {
    // Ensure buffers are valid
    if (source.getNumChannels() < 1 || output.getNumChannels() < 2) {
        mLogger.error("Invalid buffer configuration");
        return;
    }

    // Clear output buffer and prepare overlap counters
    output.clear();
    mOverlapCounts.resize(numSamples);
    std::fill(mOverlapCounts.begin(), mOverlapCounts.end(), 0.0f);

    // Calculate time between grains based on density and overlap
    float samplesPerGrain = mSampleRate / (mCloudParams.density * mCloudParams.overlap);
    mGrainCounter += numSamples;

    // Trigger new grains based on density and overlap
    while (mGrainCounter >= samplesPerGrain) {
        triggerGrain(source);
        mGrainCounter -= samplesPerGrain;
    }

    // Process all active grains
    processActiveGrains(source, output, numSamples);

    // Apply overlap normalization
    normalizeOverlaps(output, numSamples);

    // Update statistics
    updateStats();
}

void GrainCloud::triggerGrain(const AudioBuffer& source) {
    // Find an inactive grain
    auto it = std::find_if(mGrains.begin(), mGrains.end(),
                          [](const GrainParameters& g) { return !g.active; });

    if (it == mGrains.end()) {
        return; // No inactive grains available
    }

    // Calculate randomized parameters
    float positionVar = mRandomization.positionVariation * (mDistribution(mGenerator) * 2.0f - 1.0f);
    float sizeVar = mRandomization.sizeVariation * (mDistribution(mGenerator) * 2.0f - 1.0f);
    float pitchVar = mRandomization.pitchVariation * (mDistribution(mGenerator) * 2.0f - 1.0f);
    float panVar = mRandomization.panVariation * (mDistribution(mGenerator) * 2.0f - 1.0f);

    // Calculate base position within allowed range
    float basePos = mCloudParams.positionOffset +
                   (mDistribution(mGenerator) * mCloudParams.positionRange);

    // Apply position variation while respecting boundaries
    float position = std::clamp(basePos + positionVar, 0.0f, 1.0f);

    // Calculate grain size with variation (base size is 50ms)
    float grainSizeMs = 50.0f * (1.0f + sizeVar);
    size_t grainSizeSamples = static_cast<size_t>((grainSizeMs / 1000.0f) * mSampleRate);

    // Calculate stereo position with spread
    float basePan = 0.5f + (panVar * mCloudParams.spread);
    float finalPan = std::clamp(basePan, 0.0f, 1.0f);

    // Initialize grain
    it->active = true;
    it->position = static_cast<size_t>(position * source.getNumSamples());
    it->size = grainSizeSamples;
    it->currentPosition = 0;
    it->pitchRatio = std::pow(2.0f, pitchVar);  // Convert pitch variation to ratio
    it->pan = finalPan;
    it->shape = GrainShapeType::Gaussian;
    it->amplitude = 1.0f;

    // Update statistics
    mStats.totalGrainsGenerated++;
}

void GrainCloud::processActiveGrains(const AudioBuffer& source, AudioBuffer& output, size_t numSamples) {
    size_t activeCount = 0;

    // Create temporary buffer for grain processing
    AudioBuffer grainBuffer(1, numSamples);

    for (auto& grain : mGrains) {
        if (!grain.active) continue;

        activeCount++;
        grainBuffer.clear();

        // Extract grain from source
        size_t grainStart = grain.position;
        size_t grainSize = std::min(grain.size, numSamples);

        for (size_t i = 0; i < grainSize; ++i) {
            float samplePos = grainStart + (i * grain.pitchRatio);
            if (samplePos >= source.getNumSamples()) break;

            float sample = interpolateSample(source, samplePos);
            grainBuffer.write(0, &sample, 1, i);
        }

        // Apply advanced processing if needed
        if (grain.pitchRatio != 1.0f || mProcessingParams.timeStretch != 1.0f) {
            ProcessingParameters params = mProcessingParams;
            params.pitchShift = grain.pitchRatio;
            mProcessor->processGrain(grainBuffer, params);
        }

        // Apply envelope and mix to output
        for (size_t i = 0; i < numSamples; ++i) {
            if (grain.currentPosition >= grain.size) {
                grain.active = false;
                break;
            }

            float envelope = calculateEnvelope(
                static_cast<float>(grain.currentPosition) / grain.size,
                grain.shape
            );

            float sample = grainBuffer.getSample(0, i) * envelope * grain.amplitude;

            // Apply stereo positioning
            float leftGain = std::cos(grain.pan * M_PI * 0.5f);
            float rightGain = std::sin(grain.pan * M_PI * 0.5f);

            output.addSample(0, i, sample * leftGain);
            output.addSample(1, i, sample * rightGain);

            mOverlapCounts[i] += 1.0f;
            grain.currentPosition++;
        }
    }

    mStats.activeGrains = activeCount;
}

    void GrainCloud::setProcessingParameters(const ProcessingParameters& params) {
        mProcessingParams = params;

        // Log parameter changes
        mLogger.debug(("Updated processing parameters - timeStretch: " +
                      std::to_string(params.timeStretch) +
                      ", pitchShift: " + std::to_string(params.pitchShift) +
                      ", formantShift: " + std::to_string(params.formantShift)).c_str());
    }

float GrainCloud::calculateEnvelope(float phase, GrainShapeType shape) {
    switch (shape) {
        case GrainShapeType::Sine:
            return std::sin(M_PI * phase);

        case GrainShapeType::Triangle:
            return 1.0f - std::abs(2.0f * phase - 1.0f);

        case GrainShapeType::Rectangle:
            return 1.0f;

        case GrainShapeType::Gaussian:
        default: {
            float x = (phase - 0.5f) * 6.0f; // Scale for -3 to 3 range
            return std::exp(-x * x / 2.0f);
        }
    }
}

float GrainCloud::interpolateSample(const AudioBuffer& buffer, float position) {
    size_t pos1 = static_cast<size_t>(position);
    size_t pos2 = pos1 + 1;

    if (pos2 >= buffer.getNumSamples()) {
        return buffer.getSample(0, pos1);
    }

    float frac = position - pos1;
    float sample1 = buffer.getSample(0, pos1);
    float sample2 = buffer.getSample(0, pos2);

    // Cubic interpolation if we have enough points
    if (pos1 > 0 && pos2 < buffer.getNumSamples() - 1) {
        float sample0 = buffer.getSample(0, pos1 - 1);
        float sample3 = buffer.getSample(0, pos2 + 1);

        float a0 = sample3 - sample2 - sample0 + sample1;
        float a1 = sample0 - sample1 - a0;
        float a2 = sample2 - sample0;
        float a3 = sample1;

        return a0 * frac * frac * frac +
               a1 * frac * frac +
               a2 * frac +
               a3;
    }

    // Fall back to linear interpolation at boundaries
    return sample1 + frac * (sample2 - sample1);
}

void GrainCloud::normalizeOverlaps(AudioBuffer& output, size_t numSamples) {
    // Calculate average overlap for statistics
    float totalOverlap = 0.0f;

    // Apply normalization based on overlap counts
    for (size_t i = 0; i < numSamples; ++i) {
        totalOverlap += mOverlapCounts[i];

        if (mOverlapCounts[i] > 1.0f) {
            // Use square root for more natural-sounding normalization
            float gain = 1.0f / std::sqrt(mOverlapCounts[i]);

            for (size_t ch = 0; ch < output.getNumChannels(); ++ch) {
                float sample = output.getSample(ch, i) * gain;
                output.write(ch, &sample, 1, i);
            }
        }
    }

    // Update overlap statistics
    if (numSamples > 0) {
        mStats.averageOverlap = totalOverlap / numSamples;
    }
}

void GrainCloud::setCloudParameters(const CloudParameters& params) {
    // Clamp values to valid ranges
    mCloudParams.density = std::clamp(params.density, 0.1f, 1000.0f);
    mCloudParams.spread = std::clamp(params.spread, 0.0f, 1.0f);
    mCloudParams.overlap = std::clamp(params.overlap, 0.0f, 1.0f);
    mCloudParams.positionRange = std::clamp(params.positionRange, 0.0f, 1.0f);
    mCloudParams.positionOffset = std::clamp(params.positionOffset, 0.0f, 1.0f);
}

void GrainCloud::setRandomization(const RandomizationParameters& params) {
    // Clamp values to valid ranges
    mRandomization.positionVariation = std::clamp(params.positionVariation, 0.0f, 1.0f);
    mRandomization.sizeVariation = std::clamp(params.sizeVariation, 0.0f, 1.0f);
    mRandomization.pitchVariation = std::clamp(params.pitchVariation, 0.0f, 1.0f);
    mRandomization.panVariation = std::clamp(params.panVariation, 0.0f, 1.0f);
}

GrainStats GrainCloud::getStats() const {
    std::lock_guard<std::mutex> lock(mStatsMutex);
    return mStats;
}

void GrainCloud::reset() {
    // Clear all active grains
    for (auto& grain : mGrains) {
        grain.active = false;
    }

    // Reset counters and stats
    mGrainCounter = 0.0f;
    resetOverlaps();

    // Reset statistics
    std::lock_guard<std::mutex> lock(mStatsMutex);
    mStats = GrainStats{};

    mLogger.info("GrainCloud reset completed");
}

void GrainCloud::resetOverlaps() {
    std::fill(mOverlapCounts.begin(), mOverlapCounts.end(), 0.0f);
}

void GrainCloud::updateStats() {
    std::lock_guard<std::mutex> lock(mStatsMutex);

    // Update CPU usage estimation based on active grains and processing time
    float grainLoad = static_cast<float>(mStats.activeGrains) / mMaxGrains;
    mStats.cpuUsage = std::min(1.0f, grainLoad);
}

} // namespace GranularPlunderphonics