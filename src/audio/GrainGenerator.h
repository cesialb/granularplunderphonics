/**
 * @file GrainGenerator.h
 * @brief Single grain generation and shaping for granular synthesis
 */

#pragma once

#include <vector>
#include <functional>
#include "AudioBuffer.h"
#include "../common/Logger.h"
#include "../plugin/GranularParameters.h"

namespace GranularPlunderphonics {

/**
 * @struct GrainConfig
 * @brief Configuration parameters for grain generation
 */
struct GrainConfig {
    size_t position;          // Starting position in source audio (samples)
    size_t duration;          // Grain duration in samples
    GrainShapeType shape;     // Window/envelope shape
    float amplitude;          // Grain amplitude (0-1)
    bool reverse;             // Play grain in reverse if true
    float pitchShift;         // Pitch shift factor (1.0 = no shift)
};

/**
 * @class GrainGenerator
 * @brief Handles generation of individual grains for granular synthesis
 */
class GrainGenerator {
public:
    /**
     * @brief Constructor
     * @param sampleRate System sample rate
     */
    explicit GrainGenerator(double sampleRate);

    /**
     * @brief Generate a grain from source audio
     * @param source Source audio buffer
     * @param config Grain configuration
     * @return Generated grain buffer
     */
    std::shared_ptr<AudioBuffer> generateGrain(const AudioBuffer& source,
                                             const GrainConfig& config);

    /**
     * @brief Precalculate window functions for common grain sizes
     * @param minSize Minimum grain size in samples
     * @param maxSize Maximum grain size in samples
     */
    void precalculateWindows(size_t minSize, size_t maxSize);

    /**
     * @brief Get window function for a specific shape and size
     * @param shape Window shape type
     * @param size Window size in samples
     * @return Vector of window function values
     */
    std::vector<float> getWindow(GrainShapeType shape, size_t size);

private:
    double mSampleRate;
    Logger mLogger;
    bool validateWindow(const std::vector<float>& window) const;

    // Cache for window functions
    struct WindowKey {
        GrainShapeType shape;
        size_t size;

        bool operator==(const WindowKey& other) const {
            return shape == other.shape && size == other.size;
        }
    };
    struct WindowKeyHash {
        size_t operator()(const WindowKey& key) const {
            return std::hash<int>()(static_cast<int>(key.shape)) ^
                   std::hash<size_t>()(key.size);
        }
    };
    std::unordered_map<WindowKey, std::vector<float>, WindowKeyHash> mWindowCache;

    /**
     * @brief Calculate window function values
     * @param shape Window shape
     * @param size Window size in samples
     * @return Window function values
     */
    std::vector<float> calculateWindow(GrainShapeType shape, size_t size);

    /**
     * @brief Apply window function to grain
     * @param grain Grain buffer
     * @param window Window function values
     */
    void applyWindow(AudioBuffer& grain, const std::vector<float>& window);

    /**
     * @brief Interpolate sample value at a fractional position
     * @param buffer Source buffer
     * @param channel Audio channel to interpolate from
     * @param position Fractional sample position
     * @return Interpolated sample value
     */
    float interpolateSample(const AudioBuffer& buffer, size_t channel, float position);
};

} // namespace GranularPlunderphonics