/**
 * @file ModulationMatrix.cpp
 * @brief Implementation of the modulation matrix system
 */

#include "ModulationMatrix.h"
#include "../audio/ChaoticBase.h"
#include <algorithm>
#include <cmath>

namespace GranularPlunderphonics {

ModulationMatrix::ModulationMatrix(double sampleRate)
    : mSampleRate(sampleRate)
    , mLogger("ModulationMatrix")
{
    mLogger.info(("Creating ModulationMatrix with sample rate " + std::to_string(sampleRate)).c_str());
    initializePresets();
}

void ModulationMatrix::setSampleRate(double sampleRate) {
    mSampleRate = sampleRate;
    mLogger.info(("Sample rate updated to " + std::to_string(sampleRate)).c_str());
}

bool ModulationMatrix::registerSource(const std::string& id,
                                    const std::string& name,
                                    std::function<float()> valueGetter,
                                    bool isBipolar,
                                    float minValue,
                                    float maxValue)
{
    if (id.empty() || !valueGetter) {
        mLogger.error("Invalid source registration parameters");
        return false;
    }

    std::lock_guard<std::mutex> lock(mSourcesMutex);

    // Check if source already exists
    if (mSources.find(id) != mSources.end()) {
        mLogger.warn(("Source with ID '" + id + "' already exists").c_str());
        return false;
    }

    // Create and register the source
    ModulationSource source{
        .id = id,
        .name = name,
        .valueGetter = valueGetter,
        .isBipolar = isBipolar,
        .minValue = minValue,
        .maxValue = maxValue
    };

    mSources[id] = source;
    mLogger.info(("Registered modulation source: " + id).c_str());
    return true;
}

bool ModulationMatrix::registerAttractorSources(const std::string& id,
                                              const std::string& name,
                                              std::shared_ptr<ChaoticAttractor> attractor)
{
    if (!attractor) {
        mLogger.error("Null attractor pointer provided");
        return false;
    }

    size_t dimension = attractor->getDimension();
    if (dimension < 1) {
        mLogger.error(("Invalid attractor dimension: " + std::to_string(dimension)).c_str());
        return false;
    }

    bool success = true;
    const std::string dimensions = "XYZ";

    // Register each dimension as a separate source
    for (size_t i = 0; i < dimension && i < 3; ++i) {
        std::string dimId = id + "_" + dimensions[i];
        std::string dimName = name + " " + dimensions[i];

        // Create a capture of both the attractor and the dimension index
        auto valueGetter = [attractor, i]() -> float {
            std::vector<float> state = attractor->getState();
            return (i < state.size()) ? state[i] : 0.0f;
        };

        success &= registerSource(dimId, dimName, valueGetter, true, -1.0f, 1.0f);
    }

    // Optional: Register extra sources based on attractor analysis
    auto patternData = attractor->analyzePattern();

    // Register periodicity as a modulation source
    auto periodicityGetter = [attractor]() -> float {
        return attractor->analyzePattern().periodicity;
    };
    success &= registerSource(id + "_Periodicity", name + " Periodicity",
                             periodicityGetter, false, 0.0f, 1.0f);

    // Register complexity as a modulation source
    auto complexityGetter = [attractor]() -> float {
        return attractor->analyzePattern().complexity;
    };
    success &= registerSource(id + "_Complexity", name + " Complexity",
                             complexityGetter, false, 0.0f, 1.0f);

    return success;
}

bool ModulationMatrix::registerDestination(const std::string& id,
                                         const std::string& name,
                                         std::function<void(float)> valueSetter,
                                         float minValue,
                                         float maxValue,
                                         bool isAudioRate)
{
    if (id.empty() || !valueSetter) {
        mLogger.error("Invalid destination registration parameters");
        return false;
    }

    std::lock_guard<std::mutex> lock(mDestinationsMutex);

    // Check if destination already exists
    if (mDestinations.find(id) != mDestinations.end()) {
        mLogger.warn(("Destination with ID '" + id + "' already exists").c_str());
        return false;
    }

    // Create and register the destination
    ModulationDestination destination{
        .id = id,
        .name = name,
        .valueSetter = valueSetter,
        .minValue = minValue,
        .maxValue = maxValue,
        .isAudioRate = isAudioRate
    };

    mDestinations[id] = destination;
    mLogger.info(("Registered modulation destination: " + id).c_str());
    return true;
}

bool ModulationMatrix::registerParameterDestination(std::shared_ptr<Parameter> param,
                                                  bool isAudioRate)
{
    if (!param) {
        mLogger.error("Null parameter provided");
        return false;
    }

    std::string id = "param_" + std::to_string(param->getId());
    std::string name = param->getName();

    // Create a setter function that sets the parameter value
    auto valueSetter = [param](float value) {
        param->setNormalized(value);
    };

    // Find parameter range
    float minValue = 0.0f;
    float maxValue = 1.0f;

    // For float and int parameters, we can get the real min/max
    if (param->getType() == ParameterType::Float) {
        auto floatParam = std::dynamic_pointer_cast<FloatParameter>(param);
        if (floatParam) {
            minValue = floatParam->getMin();
            maxValue = floatParam->getMax();
        }
    } else if (param->getType() == ParameterType::Integer) {
        auto intParam = std::dynamic_pointer_cast<IntParameter>(param);
        if (intParam) {
            minValue = static_cast<float>(intParam->getMinInt());
            maxValue = static_cast<float>(intParam->getMaxInt());
        }
    }

    return registerDestination(id, name, valueSetter, minValue, maxValue, isAudioRate);
}

std::string ModulationMatrix::createRoute(const std::string& sourceId,
                                        const std::string& destinationId,
                                        float depth,
                                        ModulationMode mode,
                                        float offset)
{
    // Validate parameters
    depth = std::clamp(depth, 0.0f, 1.0f);
    offset = std::clamp(offset, -1.0f, 1.0f);

    std::lock_guard<std::mutex> sourceLock(mSourcesMutex);
    std::lock_guard<std::mutex> destLock(mDestinationsMutex);
    std::lock_guard<std::mutex> routeLock(mRoutesMutex);

    // Check if source and destination exist
    if (mSources.find(sourceId) == mSources.end()) {
        mLogger.error(("Source '" + sourceId + "' does not exist").c_str());
        return "";
    }

    if (mDestinations.find(destinationId) == mDestinations.end()) {
        mLogger.error(("Destination '" + destinationId + "' does not exist").c_str());
        return "";
    }

    // Check if route already exists
    if (routeExists(sourceId, destinationId)) {
        mLogger.warn(("Route from '" + sourceId + "' to '" + destinationId + "' already exists").c_str());
        return "";
    }

    // Create new route
    ModulationRoute route(sourceId, destinationId);
    route.depth.store(depth);
    route.mode.store(mode);
    route.offset.store(offset);
    route.smoothing.store(10.0f); // Default 10ms smoothing

    // Add to routes list
    mRoutes.push_back(route);

    // Update destination route indices cache
    mDestinationRouteIndices[destinationId].push_back(mRoutes.size() - 1);

    mLogger.info(("Created modulation route: " + sourceId + " -> " + destinationId).c_str());
    return sourceId + "->" + destinationId;
}

bool ModulationMatrix::removeRoute(const std::string& sourceId, const std::string& destinationId) {
    std::lock_guard<std::mutex> lock(mRoutesMutex);

    auto it = findRoute(sourceId, destinationId);
    if (it != mRoutes.end()) {
        // Remove from destination indices cache
        auto& indices = mDestinationRouteIndices[destinationId];
        size_t indexToRemove = std::distance(mRoutes.begin(), it);
        indices.erase(std::remove(indices.begin(), indices.end(), indexToRemove), indices.end());

        // Remove the route
        mRoutes.erase(it);

        mLogger.info(("Removed modulation route: " + sourceId + " -> " + destinationId).c_str());
        return true;
    }

    mLogger.warn(("Route not found: " + sourceId + " -> " + destinationId).c_str());
    return false;
}

bool ModulationMatrix::setRouteDepth(const std::string& sourceId,
                                   const std::string& destinationId,
                                   float depth)
{
    std::lock_guard<std::mutex> lock(mRoutesMutex);

    depth = std::clamp(depth, 0.0f, 1.0f);
    auto it = findRoute(sourceId, destinationId);

    if (it != mRoutes.end()) {
        it->depth.store(depth);
        return true;
    }

    return false;
}

bool ModulationMatrix::setRouteMode(const std::string& sourceId,
                                  const std::string& destinationId,
                                  ModulationMode mode)
{
    std::lock_guard<std::mutex> lock(mRoutesMutex);

    auto it = findRoute(sourceId, destinationId);

    if (it != mRoutes.end()) {
        it->mode.store(mode);
        return true;
    }

    return false;
}

bool ModulationMatrix::setRouteOffset(const std::string& sourceId,
                                    const std::string& destinationId,
                                    float offset)
{
    std::lock_guard<std::mutex> lock(mRoutesMutex);

    offset = std::clamp(offset, -1.0f, 1.0f);
    auto it = findRoute(sourceId, destinationId);

    if (it != mRoutes.end()) {
        it->offset.store(offset);
        return true;
    }

    return false;
}

bool ModulationMatrix::setRouteSmoothingTime(const std::string& sourceId,
                                           const std::string& destinationId,
                                           float smoothingMs)
{
    std::lock_guard<std::mutex> lock(mRoutesMutex);

    smoothingMs = std::max(0.0f, smoothingMs);
    auto it = findRoute(sourceId, destinationId);

    if (it != mRoutes.end()) {
        it->smoothing.store(smoothingMs);
        return true;
    }

    return false;
}

void ModulationMatrix::processControlRateModulation() {
    std::lock_guard<std::mutex> sourceLock(mSourcesMutex);
    std::lock_guard<std::mutex> destLock(mDestinationsMutex);

    // Process only non-audio-rate destinations to optimize performance
    for (const auto& [destId, destination] : mDestinations) {
        if (destination.isAudioRate) {
            continue; // Skip audio rate destinations
        }

        // Get all modulation values for this destination
        float totalModulation = getDestinationModulation(destId);

        // Apply to the destination
        destination.valueSetter(totalModulation);
    }
}

void ModulationMatrix::processAudioRateModulation(size_t sampleIndex, size_t blockSize) {
    std::lock_guard<std::mutex> sourceLock(mSourcesMutex);
    std::lock_guard<std::mutex> destLock(mDestinationsMutex);
    std::lock_guard<std::mutex> routeLock(mRoutesMutex);

    // Process only audio-rate destinations
    for (const auto& [destId, destination] : mDestinations) {
        if (!destination.isAudioRate) {
            continue; // Skip control rate destinations
        }

        // Get indices of routes affecting this destination
        auto indicesIt = mDestinationRouteIndices.find(destId);
        if (indicesIt == mDestinationRouteIndices.end() || indicesIt->second.empty()) {
            continue; // No routes for this destination
        }

        float totalModulation = 0.0f;

        // Process each route for this destination
        for (size_t routeIndex : indicesIt->second) {
            if (routeIndex >= mRoutes.size()) {
                continue; // Skip invalid indices
            }

            auto& route = mRoutes[routeIndex];
            const auto& source = mSources[route.sourceId];

            // Get source value
            float sourceValue = source.valueGetter();

            // Apply modulation mode
            float processedValue = applyModulationMode(sourceValue, route.mode);

            // Apply depth and offset
            processedValue = processedValue * route.depth + route.offset;

            // Apply audio-rate smoothing
            float smoothedValue = applySmoothingAudio(
                route.currentValue, processedValue, route.smoothing,
                sampleIndex, blockSize);

            route.currentValue.store(smoothedValue);

            // Accumulate modulation
            totalModulation += smoothedValue;
        }

        // Scale to destination range
        totalModulation = std::clamp(totalModulation, 0.0f, 1.0f);

        // Apply to destination
        destination.valueSetter(totalModulation);
    }
}

void ModulationMatrix::resetSmoothing() {
    std::lock_guard<std::mutex> lock(mRoutesMutex);

    for (auto& route : mRoutes) {
        // Get current source value directly
        const auto& source = mSources[route.sourceId];
        float sourceValue = source.valueGetter();

        // Apply modulation mode
        float processedValue = applyModulationMode(sourceValue, route.mode);

        // Apply depth and offset
        float value = processedValue * route.depth + route.offset;

        // Reset the smoothing state
        route.currentValue.store(value);
    }

    mLogger.info("Smoothing reset for all modulation routes");
}

std::vector<ModulationSource> ModulationMatrix::getAllSources() const {
    std::lock_guard<std::mutex> lock(mSourcesMutex);

    std::vector<ModulationSource> sources;
    sources.reserve(mSources.size());

    for (const auto& [id, source] : mSources) {
        sources.push_back(source);
    }

    return sources;
}

std::vector<ModulationDestination> ModulationMatrix::getAllDestinations() const {
    std::lock_guard<std::mutex> lock(mDestinationsMutex);

    std::vector<ModulationDestination> destinations;
    destinations.reserve(mDestinations.size());

    for (const auto& [id, destination] : mDestinations) {
        destinations.push_back(destination);
    }

    return destinations;
}

std::vector<ModulationRoute> ModulationMatrix::getAllRoutes() const {
    std::lock_guard<std::mutex> lock(mRoutesMutex);

    return mRoutes; // Return a copy
}

float ModulationMatrix::getDestinationModulation(const std::string& destinationId) const {
    // No need for locking here since we're using atomic values
    // and the caller should handle thread safety for the route lookup

    float totalModulation = 0.0f;

    // Get indices of routes affecting this destination
    auto indicesIt = mDestinationRouteIndices.find(destinationId);
    if (indicesIt != mDestinationRouteIndices.end()) {
        // Process each route for this destination
        for (size_t routeIndex : indicesIt->second) {
            if (routeIndex >= mRoutes.size()) {
                continue; // Skip invalid indices
            }

            const auto& route = mRoutes[routeIndex];
            const auto sourceIt = mSources.find(route.sourceId);

            if (sourceIt != mSources.end()) {
                const auto& source = sourceIt->second;

                // Get source value
                float sourceValue = source.valueGetter();

                // Apply modulation mode
                float processedValue = applyModulationMode(sourceValue, route.mode);

                // Apply depth and offset
                float value = processedValue * route.depth + route.offset;

                // Apply control-rate smoothing
                float smoothedValue = applySmoothingControl(
                    route.currentValue, value, route.smoothing);

                const_cast<ModulationRoute&>(route).currentValue.store(smoothedValue);

                // Accumulate modulation
                totalModulation += smoothedValue;
            }
        }
    }

    // Clamp final value to [0, 1] range
    return std::clamp(totalModulation, 0.0f, 1.0f);
}

bool ModulationMatrix::routeExists(const std::string& sourceId, const std::string& destinationId) const {
    std::lock_guard<std::mutex> lock(mRoutesMutex);
    return findRoute(sourceId, destinationId) != mRoutes.end();
}

bool ModulationMatrix::createPreset(const std::string& presetName) {
    std::lock_guard<std::mutex> routeLock(mRoutesMutex);
    std::lock_guard<std::mutex> presetLock(mPresetsMutex);

    if (mRoutes.empty()) {
        mLogger.warn("Cannot create preset with no active routes");
        return false;
    }

    mPresets[presetName] = mRoutes;
    mLogger.info(("Created preset: " + presetName).c_str());
    return true;
}

bool ModulationMatrix::loadPreset(const std::string& presetName) {
    std::lock_guard<std::mutex> routeLock(mRoutesMutex);
    std::lock_guard<std::mutex> presetLock(mPresetsMutex);

    auto it = mPresets.find(presetName);
    if (it == mPresets.end()) {
        mLogger.warn(("Preset not found: " + presetName).c_str());
        return false;
    }

    // Replace current routes with preset routes
    mRoutes = it->second;

    // Rebuild destination route indices cache
    mDestinationRouteIndices.clear();
    for (size_t i = 0; i < mRoutes.size(); ++i) {
        const auto& route = mRoutes[i];
        mDestinationRouteIndices[route.destinationId].push_back(i);
    }

    mLogger.info(("Loaded preset: " + presetName).c_str());
    return true;
}

    std::vector<ModulationRoute>::iterator
    ModulationMatrix::findRoute(const std::string& sourceId, const std::string& destinationId) {
    return std::find_if(mRoutes.begin(), mRoutes.end(),
        [&](const ModulationRoute& route) {
            return route.sourceId == sourceId && route.destinationId == destinationId;
        });
}

std::vector<ModulationRoute>::const_iterator
ModulationMatrix::findRoute(const std::string& sourceId, const std::string& destinationId) const {
    return std::find_if(mRoutes.begin(), mRoutes.end(),
        [&](const ModulationRoute& route) {
            return route.sourceId == sourceId && route.destinationId == destinationId;
        });
}

float ModulationMatrix::applyModulationMode(float value, ModulationMode mode) const {
    switch (mode) {
        case ModulationMode::Bipolar:
            // Keep as is, ensure in range [-1, 1]
            return std::clamp(value, -1.0f, 1.0f);

        case ModulationMode::Unipolar:
            // Convert to range [0, 1]
            return std::clamp(value * 0.5f + 0.5f, 0.0f, 1.0f);

        case ModulationMode::AbsBipolar:
            // Take absolute value of bipolar signal
            return std::clamp(std::abs(value), 0.0f, 1.0f);

        default:
            return value;
    }
}

float ModulationMatrix::applySmoothingControl(float currentValue, float targetValue, float smoothingMs) const {
    if (smoothingMs <= 0.0f) {
        return targetValue; // No smoothing
    }

    // Convert smoothing time to coefficient
    // Assuming control rate is ~2ms (500Hz)
    float alpha = 1.0f - std::exp(-2.0f / smoothingMs);

    // Apply first-order lowpass filter
    return currentValue + alpha * (targetValue - currentValue);
}

float ModulationMatrix::applySmoothingAudio(float currentValue, float targetValue,
                                          float smoothingMs, size_t sampleIndex, size_t blockSize) {
    if (smoothingMs <= 0.0f) {
        return targetValue; // No smoothing
    }

    // Convert smoothing time to coefficient
    float alpha = 1.0f - std::exp(-1000.0f / (smoothingMs * mSampleRate));

    // Apply smoothing based on position in block
    float progress = static_cast<float>(sampleIndex) / blockSize;
    float smoothingAmount = alpha * progress;

    // Apply partial smoothing
    return currentValue + smoothingAmount * (targetValue - currentValue);
}

void ModulationMatrix::initializePresets() {
    std::lock_guard<std::mutex> lock(mPresetsMutex);

    // Default empty presets - will be populated when sources/destinations are registered
    mPresets.clear();

    mLogger.info("Initialized modulation matrix presets");
}

} // namespace GranularPlunderphonics