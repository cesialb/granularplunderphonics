/**
 * @file GrainCloud.h
 * @brief Advanced grain cloud management system for granular synthesis
 */

#pragma once

#include <vector>
#include <random>
#include <mutex>
#include "AudioBuffer.h"
#include "GrainGenerator.h"
#include "GrainProcessor.h"

namespace GranularPlunderphonics {

/**
 * @struct GrainParameters
 * @brief Parameters for individual grain processing
 */
struct GrainParameters {
    bool active{false};             // Whether grain is currently playing
    size_t position{0};            // Position in source audio
    size_t size{0};               // Size in samples
    size_t currentPosition{0};    // Current playback position
    float amplitude{1.0f};       // Amplitude scaling
    float pitchRatio{1.0f};     // Pitch shift ratio
    float pan{0.5f};           // Stereo position (0 = left, 1 = right)
    GrainShapeType shape{GrainShapeType::Gaussian}; // Envelope shape
};

/**
 * @struct CloudParameters
 * @brief Configuration parameters for grain cloud behavior
 */
struct CloudParameters {
    float density{10.0f};           // Grains per second
    float spread{0.5f};             // Stereo spread amount
    float overlap{0.5f};            // Grain overlap factor
    float positionRange{1.0f};      // Range of source material to use (0-1)
    float positionOffset{0.0f};     // Starting position in source (0-1)
};

/**
 * @struct RandomizationParameters
 * @brief Parameters controlling grain randomization
 */
struct RandomizationParameters {
    float positionVariation{0.1f};    // Random position variance
    float sizeVariation{0.1f};        // Random size variance
    float pitchVariation{0.1f};       // Random pitch variance
    float panVariation{0.1f};         // Random pan variance
};

/**
 * @struct GrainStats
 * @brief Statistics about grain cloud processing
 */
struct GrainStats {
    size_t activeGrains{0};          // Currently active grains
    size_t totalGrainsGenerated{0};  // Total grains generated
    float averageOverlap{0.0f};      // Average grain overlap
    float cpuUsage{0.0f};            // CPU usage estimate
};

/**
 * @class GrainCloud
 * @brief Manages multiple concurrent grains for granular synthesis
 */
class GrainCloud {
public:
    /**
     * @brief Constructor
     * @param maxGrains Maximum number of simultaneous grains
     * @param sampleRate System sample rate
     */
    explicit GrainCloud(size_t maxGrains = 100, double sampleRate = 44100.0);

    /**
     * @brief Process audio through the grain cloud
     * @param source Source audio buffer
     * @param output Output audio buffer
     * @param numSamples Number of samples to process
     */
    void process(const AudioBuffer& source, AudioBuffer& output, size_t numSamples);

    /**
     * @brief Set cloud parameters
     * @param params Cloud behavior parameters
     */
    void setCloudParameters(const CloudParameters& params);

    /**
     * @brief Set randomization parameters
     * @param params Randomization control parameters
     */
    void setRandomization(const RandomizationParameters& params);

    /**
     * @brief Get current grain cloud statistics
     * @return Current statistics
     */
    GrainStats getStats() const;

    /**
     * @brief Reset the grain cloud state
     */
    void reset();

    /**
     * @brief Set the advanced processing parameters for grain manipulation
     * @param params Processing parameters controlling time stretch and pitch shift
     */
    void setProcessingParameters(const ProcessingParameters& params);

private:

    /**
     * @brief Trigger a new grain
     * @param source Source audio buffer
     */
    void triggerGrain(const AudioBuffer& source);

    /**
     * @brief Process all active grains
     * @param source Source audio buffer
     * @param output Output buffer
     * @param numSamples Number of samples to process
     */
    void processActiveGrains(const AudioBuffer& source, AudioBuffer& output, size_t numSamples);

    /**
     * @brief Calculate envelope value for a given phase
     * @param phase Phase position (0-1)
     * @param shape Envelope shape type
     * @return Envelope amplitude
     */
    float calculateEnvelope(float phase, GrainShapeType shape);

    /**
     * @brief Interpolate sample at a fractional position
     * @param buffer Source buffer
     * @param position Fractional sample position
     * @return Interpolated sample value
     */
    float interpolateSample(const AudioBuffer& buffer, float position);

    /**
     * @brief Reset overlap counters
     */
    void resetOverlaps();

    /**
     * @brief Apply overlap normalization
     * @param output Output buffer to normalize
     * @param numSamples Number of samples
     */
    void normalizeOverlaps(AudioBuffer& output, size_t numSamples);

    /**
     * @brief Update grain cloud statistics
     */
    void updateStats();

    // Configuration
    size_t mMaxGrains;
    double mSampleRate;
    CloudParameters mCloudParams;
    RandomizationParameters mRandomization;

    // Grain management
    std::vector<GrainParameters> mGrains;
    std::vector<float> mOverlapCounts;
    float mGrainCounter{0.0f};

    // Statistics
    GrainStats mStats;

    // Random number generation
    std::mt19937 mGenerator;
    std::uniform_real_distribution<float> mDistribution;

    // Synchronization
    mutable std::mutex mStatsMutex;
    Logger mLogger;

    std::unique_ptr<GrainProcessor> mProcessor;
    ProcessingParameters mProcessingParams;};

} // namespace GranularPlunderphonics