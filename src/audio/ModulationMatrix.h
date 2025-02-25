/**
 * @file ModulationMatrix.h
 * @brief Flexible modulation matrix for routing chaotic attractor outputs to synthesis parameters
 */

#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <memory>
#include <mutex>
#include <atomic>
#include "../common/Logger.h"
#include "../plugin/ParameterManager.h"

namespace GranularPlunderphonics {

// Forward declarations
class ChaoticAttractor;

/**
 * @enum ModulationMode
 * @brief Defines how modulation signals are processed
 */
enum class ModulationMode {
    Bipolar,    // -1.0 to 1.0
    Unipolar,   // 0.0 to 1.0
    AbsBipolar  // Absolute value of bipolar (-1.0 to 1.0 becomes 0.0 to 1.0)
};

/**
 * @struct ModulationSource
 * @brief Defines a modulation source
 */
struct ModulationSource {
    std::string id;                      // Unique identifier
    std::string name;                    // Display name
    std::function<float()> valueGetter;  // Function to get current value
    bool isBipolar{true};                // Whether the source outputs bipolar values
    float minValue{-1.0f};               // Minimum possible value
    float maxValue{1.0f};                // Maximum possible value
};

/**
 * @struct ModulationDestination
 * @brief Defines a modulation destination
 */
struct ModulationDestination {
    std::string id;                      // Unique identifier
    std::string name;                    // Display name
    std::function<void(float)> valueSetter; // Function to set modulated value
    float minValue{0.0f};                // Minimum parameter value
    float maxValue{1.0f};                // Maximum parameter value
    bool isAudioRate{false};             // Whether parameter changes at audio rate
};

/**
 * @struct ModulationRoute
 * @brief Defines a connection between a source and destination
 */
struct ModulationRoute {
    std::string sourceId;            // Source identifier
    std::string destinationId;       // Destination identifier
    std::atomic<float> depth{0.5f};  // Modulation amount (0.0 to 1.0)
    std::atomic<float> offset{0.0f}; // Modulation center point (-1.0 to 1.0)
    std::atomic<float> smoothing{10.0f}; // Milliseconds of smoothing
    std::atomic<ModulationMode> mode{ModulationMode::Bipolar}; // Processing mode

    // Smoothing state
    std::atomic<float> currentValue{0.0f}; // Current smoothed value

    // Constructor with initialization
    ModulationRoute(const std::string& src, const std::string& dest)
        : sourceId(src), destinationId(dest) {}

    ModulationRoute(const ModulationRoute& other)
            : sourceId(other.sourceId), destinationId(other.destinationId) {
        depth.store(other.depth.load());
        offset.store(other.offset.load());
        smoothing.store(other.smoothing.load());
        mode.store(other.mode.load());
        currentValue.store(other.currentValue.load());
    }

    ModulationRoute& operator=(const ModulationRoute& other) {
        if (this != &other) {
            sourceId = other.sourceId;
            destinationId = other.destinationId;
            depth.store(other.depth.load());
            offset.store(other.offset.load());
            smoothing.store(other.smoothing.load());
            mode.store(other.mode.load());
            currentValue.store(other.currentValue.load());
        }
        return *this;
    }
};

/**
 * @class ModulationMatrix
 * @brief Manages routing between modulation sources and synthesis parameter destinations
 */
class ModulationMatrix {
public:
    /**
     * @brief Constructor
     * @param sampleRate The system sample rate for smoothing calculations
     */
    explicit ModulationMatrix(double sampleRate = 44100.0);

    /**
     * @brief Update sample rate for smoothing calculations
     * @param sampleRate New sample rate in Hz
     */
    void setSampleRate(double sampleRate);

    /**
     * @brief Register a modulation source
     * @param id Unique identifier
     * @param name Display name
     * @param valueGetter Function to get current value
     * @param isBipolar Whether source produces bipolar values
     * @param minValue Minimum possible value
     * @param maxValue Maximum possible value
     * @return True if registration succeeded
     */
    bool registerSource(const std::string& id,
                      const std::string& name,
                      std::function<float()> valueGetter,
                      bool isBipolar = true,
                      float minValue = -1.0f,
                      float maxValue = 1.0f);

    /**
     * @brief Register modulation sources from a chaotic attractor
     * @param id Base identifier for the attractor
     * @param name Base display name
     * @param attractor Pointer to the chaotic attractor
     * @return True if registration succeeded
     */
    bool registerAttractorSources(const std::string& id,
                                const std::string& name,
                                std::shared_ptr<ChaoticAttractor> attractor);

    /**
     * @brief Register a modulation destination
     * @param id Unique identifier
     * @param name Display name
     * @param valueSetter Function to set modulated value
     * @param minValue Minimum parameter value
     * @param maxValue Maximum parameter value
     * @param isAudioRate Whether parameter changes at audio rate
     * @return True if registration succeeded
     */
    bool registerDestination(const std::string& id,
                           const std::string& name,
                           std::function<void(float)> valueSetter,
                           float minValue = 0.0f,
                           float maxValue = 1.0f,
                           bool isAudioRate = false);

    /**
     * @brief Register a parameter as a modulation destination
     * @param param Shared pointer to parameter
     * @param isAudioRate Whether parameter changes at audio rate
     * @return True if registration succeeded
     */
    bool registerParameterDestination(std::shared_ptr<Parameter> param,
                                    bool isAudioRate = false);

    /**
     * @brief Create a modulation route between source and destination
     * @param sourceId Source identifier
     * @param destinationId Destination identifier
     * @param depth Initial modulation depth (0.0 to 1.0)
     * @param mode Initial modulation mode
     * @param offset Initial offset value (-1.0 to 1.0)
     * @return Route ID if successful, empty string if failed
     */
    std::string createRoute(const std::string& sourceId,
                          const std::string& destinationId,
                          float depth = 0.5f,
                          ModulationMode mode = ModulationMode::Bipolar,
                          float offset = 0.0f);

    /**
     * @brief Remove a modulation route
     * @param sourceId Source identifier
     * @param destinationId Destination identifier
     * @return True if route was removed
     */
    bool removeRoute(const std::string& sourceId, const std::string& destinationId);

    /**
     * @brief Set depth of a modulation route
     * @param sourceId Source identifier
     * @param destinationId Destination identifier
     * @param depth New depth value (0.0 to 1.0)
     * @return True if depth was set
     */
    bool setRouteDepth(const std::string& sourceId,
                      const std::string& destinationId,
                      float depth);

    /**
     * @brief Set mode of a modulation route
     * @param sourceId Source identifier
     * @param destinationId Destination identifier
     * @param mode New modulation mode
     * @return True if mode was set
     */
    bool setRouteMode(const std::string& sourceId,
                     const std::string& destinationId,
                     ModulationMode mode);

    /**
     * @brief Set offset of a modulation route
     * @param sourceId Source identifier
     * @param destinationId Destination identifier
     * @param offset New offset value (-1.0 to 1.0)
     * @return True if offset was set
     */
    bool setRouteOffset(const std::string& sourceId,
                       const std::string& destinationId,
                       float offset);

    /**
     * @brief Set smoothing time of a modulation route
     * @param sourceId Source identifier
     * @param destinationId Destination identifier
     * @param smoothingMs Smoothing time in milliseconds
     * @return True if smoothing was set
     */
    bool setRouteSmoothingTime(const std::string& sourceId,
                              const std::string& destinationId,
                              float smoothingMs);

    /**
     * @brief Process all modulation routes for control rate parameters
     * This should be called once per processing block
     */
    void processControlRateModulation();

    /**
     * @brief Process all modulation routes for audio rate parameters
     * @param sampleIndex The current sample index within the block
     * @param blockSize The total block size
     * This should be called for each sample in the audio processing loop
     */
    void processAudioRateModulation(size_t sampleIndex, size_t blockSize);

    /**
     * @brief Reset all modulation smoothing
     * Call this when audio processing is restarted
     */
    void resetSmoothing();

    /**
     * @brief Get all modulation sources
     * @return Vector of modulation sources
     */
    std::vector<ModulationSource> getAllSources() const;

    /**
     * @brief Get all modulation destinations
     * @return Vector of modulation destinations
     */
    std::vector<ModulationDestination> getAllDestinations() const;

    /**
     * @brief Get all modulation routes
     * @return Vector of modulation routes
     */
    std::vector<ModulationRoute> getAllRoutes() const;

    /**
     * @brief Get current modulation amount for a destination
     * @param destinationId Destination identifier
     * @return Total modulation value
     */
    float getDestinationModulation(const std::string& destinationId) const;

    /**
     * @brief Check if a route exists
     * @param sourceId Source identifier
     * @param destinationId Destination identifier
     * @return True if route exists
     */
    bool routeExists(const std::string& sourceId, const std::string& destinationId) const;

    /**
     * @brief Create a preset with common modulation configurations
     * @param presetName Name of the preset configuration
     * @return True if preset was created successfully
     */
    bool createPreset(const std::string& presetName);

    /**
     * @brief Load a preset modulation configuration
     * @param presetName Name of the preset configuration
     * @return True if preset was loaded successfully
     */
    bool loadPreset(const std::string& presetName);

private:
    double mSampleRate;
    std::unordered_map<std::string, ModulationSource> mSources;
    std::unordered_map<std::string, ModulationDestination> mDestinations;
    std::vector<ModulationRoute> mRoutes;

    // Cache lookup for improved performance
    std::unordered_map<std::string, std::vector<size_t>> mDestinationRouteIndices;

    // Find a route by source and destination IDs
    std::vector<ModulationRoute>::iterator findRoute(const std::string& sourceId,
                                                   const std::string& destinationId);
    std::vector<ModulationRoute>::const_iterator findRoute(const std::string& sourceId,
                                                         const std::string& destinationId) const;

    // Apply modulation mode transformation
    float applyModulationMode(float value, ModulationMode mode) const;

    // Apply smoothing to a value change
    float applySmoothingControl(float currentValue, float targetValue, float smoothingMs) const;
    float applySmoothingAudio(float currentValue, float targetValue, float smoothingMs,
                            size_t sampleIndex, size_t blockSize);

    // Presets for common modulation configurations
    std::unordered_map<std::string, std::vector<ModulationRoute>> mPresets;

    // Thread safety
    mutable std::mutex mSourcesMutex;
    mutable std::mutex mDestinationsMutex;
    mutable std::mutex mRoutesMutex;
    mutable std::mutex mPresetsMutex;

    // Logging
    Logger mLogger{"ModulationMatrix"};

    // Initialize common presets
    void initializePresets();
};

} // namespace GranularPlunderphonics