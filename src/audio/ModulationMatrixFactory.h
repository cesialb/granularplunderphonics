/**
 * @file ModulationMatrixFactory.h
 * @brief Factory for creating and initializing ModulationMatrix with common configurations
 */

#pragma once

#include "ModulationMatrix.h"
#include "../plugin/GranularParameters.h"
#include "../audio/LorenzAttractor.h"
#include "../audio/GrainCloud.h"
#include <memory>

namespace GranularPlunderphonics {

/**
 * @class ModulationMatrixFactory
 * @brief Creates and configures modulation matrix setups
 */
class ModulationMatrixFactory {
public:
    /**
     * @brief Create a standard modulation matrix with common sources and destinations
     * @param paramManager Parameter manager containing parameters to modulate
     * @param attractors Map of chaotic attractors to use as modulation sources
     * @param cloudParams Cloud parameters to expose as modulation destinations
     * @param sampleRate System sample rate
     * @return Configured modulation matrix
     */
    static std::shared_ptr<ModulationMatrix> createStandardMatrix(
        ParameterManager& paramManager,
        const std::map<std::string, std::shared_ptr<ChaoticAttractor>>& attractors,
        CloudParameters& cloudParams,
        double sampleRate = 44100.0
    ) {
        auto matrix = std::make_shared<ModulationMatrix>(sampleRate);

        // Register attractor sources
        for (const auto& [id, attractor] : attractors) {
            matrix->registerAttractorSources(id, id, attractor);
        }

        // Register parameter destinations
        auto parameters = paramManager.getAllParameters();
        for (auto& param : parameters) {
            matrix->registerParameterDestination(param);
        }

        // Register cloud parameter destinations
        registerCloudParameters(*matrix, cloudParams);

        // Create default presets
        createDefaultPresets(*matrix, attractors, parameters);

        return matrix;
    }

private:
    /**
     * @brief Register cloud parameters as modulation destinations
     * @param matrix Modulation matrix to register with
     * @param cloudParams Cloud parameters to register
     */
    static void registerCloudParameters(ModulationMatrix& matrix, CloudParameters& cloudParams) {
        // Register cloud density
        matrix.registerDestination(
            "cloud_density", "Grain Density",
            [&cloudParams](float value) {
                cloudParams.density = 0.1f + value * 99.9f; // 0.1 to 100 Hz
            },
            0.1f, 100.0f
        );

        // Register cloud spread
        matrix.registerDestination(
            "cloud_spread", "Stereo Spread",
            [&cloudParams](float value) {
                cloudParams.spread = value;
            },
            0.0f, 1.0f
        );

        // Register position range
        matrix.registerDestination(
            "cloud_position_range", "Position Range",
            [&cloudParams](float value) {
                cloudParams.positionRange = value;
            },
            0.0f, 1.0f
        );

        // Register position offset
        matrix.registerDestination(
            "cloud_position_offset", "Position Offset",
            [&cloudParams](float value) {
                cloudParams.positionOffset = value;
            },
            0.0f, 1.0f
        );
    }

    /**
     * @brief Create default presets for the modulation matrix
     * @param matrix Modulation matrix to configure
     * @param attractors Available chaotic attractors
     * @param parameters Available plugin parameters
     */
    static void createDefaultPresets(
        ModulationMatrix& matrix,
        const std::map<std::string, std::shared_ptr<ChaoticAttractor>>& attractors,
        const std::vector<std::shared_ptr<Parameter>>& parameters
    ) {
        // If no attractors, can't create presets
        if (attractors.empty()) return;

        // Get first attractor for default presets
        auto firstAttractorId = attractors.begin()->first;

        // Find grain size parameter
        auto grainSizeParam = std::find_if(parameters.begin(), parameters.end(),
            [](const std::shared_ptr<Parameter>& param) {
                return param->getId() == kGrainSizeId;
            });

        // Find grain density parameter
        auto grainDensityParam = std::find_if(parameters.begin(), parameters.end(),
            [](const std::shared_ptr<Parameter>& param) {
                return param->getId() == kGrainDensityId;
            });

        // Find grain shape parameter
        auto grainShapeParam = std::find_if(parameters.begin(), parameters.end(),
            [](const std::shared_ptr<Parameter>& param) {
                return param->getId() == kGrainShapeId;
            });

        // Create routes for "Chaotic Size" preset
        if (grainSizeParam != parameters.end()) {
            std::string paramId = "param_" + std::to_string((*grainSizeParam)->getId());

            // X dimension affects grain size
            matrix.createRoute(
                firstAttractorId + "_X",
                paramId,
                0.7f, // Depth
                ModulationMode::Bipolar,
                0.0f  // Offset
            );
        }

        // Create routes for "Chaotic Density" preset
        if (grainDensityParam != parameters.end()) {
            std::string paramId = "param_" + std::to_string((*grainDensityParam)->getId());

            // X dimension affects grain density
            matrix.createRoute(
                firstAttractorId + "_Y",
                paramId,
                0.5f, // Depth
                ModulationMode::Unipolar,
                0.2f  // Offset
            );
        }

        // Create routes for cloud position
        matrix.createRoute(
            firstAttractorId + "_Z",
            "cloud_position_offset",
            0.3f, // Depth
            ModulationMode::AbsBipolar,
            0.5f  // Center position
        );

        // Save as default preset
        matrix.createPreset("Default");

        // Create "Wild Chaos" preset with more extreme settings
        if (grainSizeParam != parameters.end() && grainDensityParam != parameters.end()) {
            std::string sizeParamId = "param_" + std::to_string((*grainSizeParam)->getId());
            std::string densityParamId = "param_" + std::to_string((*grainDensityParam)->getId());

            // Remove existing routes
            matrix.removeRoute(firstAttractorId + "_X", sizeParamId);
            matrix.removeRoute(firstAttractorId + "_Y", densityParamId);
            matrix.removeRoute(firstAttractorId + "_Z", "cloud_position_offset");

            // Create new routes with more extreme settings
            matrix.createRoute(firstAttractorId + "_X", sizeParamId, 1.0f, ModulationMode::Bipolar, 0.0f);
            matrix.createRoute(firstAttractorId + "_Y", densityParamId, 0.8f, ModulationMode::Unipolar, 0.1f);
            matrix.createRoute(firstAttractorId + "_Z", "cloud_position_offset", 0.9f, ModulationMode::Bipolar, 0.5f);
            matrix.createRoute(firstAttractorId + "_Complexity", "cloud_spread", 1.0f, ModulationMode::Unipolar, 0.0f);

            // Save as preset
            matrix.createPreset("Wild Chaos");
        }

        // Create "Subtle Motion" preset with more gentle settings
        if (grainSizeParam != parameters.end() && grainDensityParam != parameters.end()) {
            std::string sizeParamId = "param_" + std::to_string((*grainSizeParam)->getId());
            std::string densityParamId = "param_" + std::to_string((*grainDensityParam)->getId());

            // Remove existing routes again
            matrix.removeRoute(firstAttractorId + "_X", sizeParamId);
            matrix.removeRoute(firstAttractorId + "_Y", densityParamId);
            matrix.removeRoute(firstAttractorId + "_Z", "cloud_position_offset");
            matrix.removeRoute(firstAttractorId + "_Complexity", "cloud_spread");

            // Create new routes with gentle settings
            matrix.createRoute(firstAttractorId + "_X", sizeParamId, 0.2f, ModulationMode::Unipolar, 0.5f);
            matrix.createRoute(firstAttractorId + "_Y", densityParamId, 0.15f, ModulationMode::Unipolar, 0.4f);
            matrix.createRoute(firstAttractorId + "_Z", "cloud_position_offset", 0.1f, ModulationMode::AbsBipolar, 0.3f);

            // Save as preset
            matrix.createPreset("Subtle Motion");

            // Restore default routes
            matrix.loadPreset("Default");
        }
    }
};

} // namespace GranularPlunderphonics