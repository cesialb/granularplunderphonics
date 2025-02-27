/**
 * @file GranularParameters.h
 * @brief Parameter definitions for the GranularPlunderphonics plugin
 */

#pragma once

#include "ParameterManager.h"

namespace GranularPlunderphonics {

/**
 * @enum GranularParameterID
 * @brief Parameter IDs for GranularPlunderphonics plugin
 */
enum GranularParameterID : ParamID {
    // Global parameters
    kBypassId = 1000,

    // Granular synthesis parameters
    kGrainSizeId = 2000,
    kGrainShapeId = 2001,
    kGrainDensityId = 2002,

    // Add more parameters here as needed
};

/**
 * @enum GrainShapeType
 * @brief Grain envelope shapes
 */
enum class GrainShapeType {
    Sine = 0,
    Triangle = 1,
    Rectangle = 2,
    Gaussian = 3
};

/**
 * @class GranularParameters
 * @brief Factory and container for granular synthesis parameters
 */
class GranularParameters {
public:
    /**
     * @brief Create and register all granular synthesis parameters
     * @param paramManager Parameter manager to register with
     * @return True if all parameters registered successfully
     */
    static bool registerParameters(ParameterManager& paramManager) {
        Logger logger("GranularParameters");
        logger.info("Registering granular synthesis parameters");

        bool success = true;

        // Bypass parameter
        success &= paramManager.registerParameter(createBypass());

        // Grain size parameter (1-100ms, default 50ms, linear)
        success &= paramManager.registerParameter(createGrainSize());

        // Grain shape parameter (enum: Sine, Triangle, Rectangle, Gaussian)
        success &= paramManager.registerParameter(createGrainShape());

        // Grain density parameter (0.1-100Hz, default 10Hz, logarithmic)
        success &= paramManager.registerParameter(createGrainDensity());

        logger.info(std::string("Parameter registration ") +
            (success ? "succeeded" : "failed"));

        return success;
    }

    /**
     * @brief Create bypass parameter
     * @return Parameter pointer
     */
    static std::shared_ptr<BoolParameter> createBypass() {
        return std::make_shared<BoolParameter>(
            kBypassId,
            "Bypass",
            "Byp",
            false,
            ParameterFlags::IsBypass
        );
    }

    /**
     * @brief Create grain size parameter
     * @return Parameter pointer
     */
    static std::shared_ptr<FloatParameter> createGrainSize() {
        return std::make_shared<FloatParameter>(
            kGrainSizeId,
            "Grain Size",
            "Size",
            1.0f,     // Min: 1ms
            100.0f,   // Max: 100ms
            50.0f,    // Default: 50ms
            ParameterFlags::NoFlags,
            "ms",
            20.0f     // 20ms smoothing time
        );
    }

    /**
     * @brief Create grain shape parameter
     * @return Parameter pointer
     */
    static std::shared_ptr<EnumParameter> createGrainShape() {
        std::vector<EnumValue> shapes = {
            {static_cast<int>(GrainShapeType::Sine),     "Sine",     "Sin"},
            {static_cast<int>(GrainShapeType::Triangle), "Triangle", "Tri"},
            {static_cast<int>(GrainShapeType::Rectangle),"Rectangle","Rect"},
            {static_cast<int>(GrainShapeType::Gaussian), "Gaussian", "Gauss"}
        };

        return std::make_shared<EnumParameter>(
            kGrainShapeId,
            "Grain Shape",
            "Shape",
            shapes,
            static_cast<int>(GrainShapeType::Gaussian)  // Default: Gaussian
        );
    }

    /**
     * @brief Create grain density parameter
     * @return Parameter pointer
     */
    static std::shared_ptr<FloatParameter> createGrainDensity() {
        return std::make_shared<FloatParameter>(
            kGrainDensityId,
            "Grain Density",
            "Density",
            0.1f,     // Min: 0.1Hz
            100.0f,   // Max: 100Hz
            10.0f,    // Default: 10Hz
            ParameterFlags::IsLogarithmic,
            "Hz",
            50.0f     // 50ms smoothing time
        );
    }
};

} // namespace GranularPlunderphonics