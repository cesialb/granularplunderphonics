/**
 * @file GrainEngine.h
 * @brief Enhanced granular synthesis engine header
 */

#pragma once

#include <vector>
#include <mutex>
#include <random>
#include <unordered_map>
#include "AudioBuffer.h"
#include "../common/Logger.h"
#include "../plugin/GranularParameters.h"

namespace GranularPlunderphonics {

/**
 * @struct Grain
 * @brief Represents a single grain in the granular synthesis engine
 */
struct Grain {
    bool active{false};             // Whether grain is currently playing
    size_t startPosition{0};        // Start position in source audio
    size_t currentPosition{0};      // Current playback position
    size_t grainSize{0};           // Size of grain in samples
    float amplitude{1.0f};         // Amplitude scaling
    float pan{0.5f};              // Stereo position (0 = left, 1 = right)
    bool reverse{false};           // Reverse playback if true
    GrainShapeType shape;          // Window/envelope shape

    Grain() : shape(GrainShapeType::Gaussian) {}
};

/**
 * @struct WindowKey
 * @brief Key for window function cache lookup
 */
struct WindowKey {
    GrainShapeType shape;
    size_t size;

    bool operator==(const WindowKey& other) const {
        return shape == other.shape && size == other.size;
    }
};

/**
 * @class GrainEngine
 * @brief Enhanced granular synthesis engine with advanced grain manipulation
 */
class GrainEngine {
public:
    /**
     * @brief Constructor
     * @param sampleRate System sample rate
     * @param maxGrains Maximum number of simultaneous grains
     */
    explicit GrainEngine(double sampleRate = 44100.0, size_t maxGrains = 100);

    /**
     * @brief Process audio through the grain engine
     * @param input Input buffer
     * @param output Output buffer
     * @param numSamples Number of samples to process
     * @return true if successful
     */
    bool process(AudioBuffer& input, AudioBuffer& output, size_t numSamples);

    /**
     * @brief Set grain size
     * @param sizeMs Grain size in milliseconds
     */
    void setGrainSize(float sizeMs);

    /**
     * @brief Set grain density
     * @param grainsPerSecond Number of grains per second
     */
    void setGrainDensity(float grainsPerSecond);

    /**
     * @brief Set grain envelope shape
     * @param shape Window function type for grain envelope
     */
    void setGrainShape(GrainShapeType shape);

    /**
     * @brief Set randomization parameters
     * @param sizeVar Grain size variation (0-1)
     * @param posVar Position variation (0-1)
     * @param panVar Stereo position variation (0-1)
     * @param revProb Probability of reverse playback (0-1)
     */
    void setRandomization(float sizeVar, float posVar, float panVar, float revProb);

private:
    // Window function handling
    void initializeWindowFunctions();
    std::vector<float> calculateWindow(GrainShapeType shape, size_t size);
    float getGrainEnvelope(const Grain& grain, size_t position);

    // Grain processing
    void processActiveGrains(const AudioBuffer& input, AudioBuffer& output, size_t sampleIndex);
    void triggerGrain(const AudioBuffer& input);
    float interpolateSample(const AudioBuffer& buffer, size_t channel, float position);
    bool validateBuffers(const AudioBuffer& input, const AudioBuffer& output, size_t numSamples);

    // Member variables
    double mSampleRate;
    size_t mMaxGrains;
    std::vector<Grain> mGrains;
    std::atomic<float> mGrainSizeMs;
    std::atomic<float> mGrainDensity;
    std::atomic<GrainShapeType> mGrainShape;

    // Randomization parameters
    float mSizeVariation{0.1f};
    float mPositionVariation{0.1f};
    float mPanVariation{0.1f};
    float mReverseProb{0.0f};

    // Random number generation
    std::mt19937 mGenerator;
    std::uniform_real_distribution<float> mPosDistribution;

    // Window cache
    struct WindowKeyHash {
        std::size_t operator()(const WindowKey& k) const {
            return std::hash<int>()(static_cast<int>(k.shape)) ^
                   std::hash<size_t>()(k.size);
        }
    };
    std::unordered_map<WindowKey, std::vector<float>, WindowKeyHash> mWindowCache;

    // Synchronization
    std::atomic<int> mNextGrainTime;
    mutable std::mutex mGrainMutex;
    Logger mLogger;
};

} // namespace GranularPlunderphonics