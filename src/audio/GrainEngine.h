/**
 * @file GrainEngine.h
 * @brief Core granular synthesis engine
 */

#pragma once

#include <vector>
#include <memory>
#include <queue>
#include <mutex>
#include <atomic>
#include "AudioBuffer.h"
#include "../common/Logger.h"
#include "../plugin/GranularParameters.h"

namespace GranularPlunderphonics {

/**
 * @struct Grain
 * @brief Represents a single grain in the granular synthesis engine
 */
struct Grain {
    size_t startPosition;      // Start position in source audio
    size_t currentPosition;    // Current playback position
    size_t grainSize;         // Size of grain in samples
    float amplitude;          // Current amplitude
    GrainShapeType shape;     // Envelope shape
    bool active;              // Whether grain is currently playing

    Grain() : startPosition(0), currentPosition(0), grainSize(0),
              amplitude(0.0f), shape(GrainShapeType::Gaussian), active(false) {}
};

/**
 * @class GrainEngine
 * @brief Manages grain generation and processing for granular synthesis
 */
class GrainEngine {
public:
    /**
     * @brief Constructor
     * @param sampleRate System sample rate
     * @param maxGrains Maximum number of simultaneous grains
     */
    GrainEngine(double sampleRate = 44100.0, size_t maxGrains = 100);

    /**
     * @brief Process audio through the grain engine
     * @param input Input buffer
     * @param output Output buffer
     * @param numSamples Number of samples to process
     * @return true if successful
     */
    bool process(const AudioBuffer& input, AudioBuffer& output, size_t numSamples);

    /**
     * @brief Set grain size
     * @param sizeMs Grain size in milliseconds
     */
    void setGrainSize(float sizeMs);

    /**
     * @brief Set grain density
     * @param grainPerSecond Number of grains per second
     */
    void setGrainDensity(float grainsPerSecond);

    /**
     * @brief Set grain shape
     * @param shape Envelope shape for new grains
     */
    void setGrainShape(GrainShapeType shape);

    /**
     * @brief Reset the engine state
     */
    void reset();

private:
    double mSampleRate;
    size_t mMaxGrains;
    std::vector<Grain> mGrains;
    std::queue<size_t> mFreeGrainIndices;
    std::atomic<float> mGrainSizeMs;
    std::atomic<float> mGrainDensity;
    std::atomic<GrainShapeType> mGrainShape;
    std::atomic<size_t> mNextGrainTime;
    mutable std::mutex mGrainMutex;
    Logger mLogger;

    /**
     * @brief Trigger a new grain
     * @param sourcePos Position in source audio to start grain
     * @return true if grain was triggered
     */
    bool triggerGrain(size_t sourcePos);

    /**
     * @brief Calculate grain envelope value
     * @param grain Current grain
     * @param position Position within grain
     * @return Envelope amplitude
     */
    float calculateEnvelope(const Grain& grain, size_t position) const;

    /**
     * @brief Update grain states
     * @param numSamples Number of samples to advance
     */
    void updateGrains(size_t numSamples);
};

} // namespace GranularPlunderphonics
