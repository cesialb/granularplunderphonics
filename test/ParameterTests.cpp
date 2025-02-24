#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "../src/plugin/ParameterManager.h"
#include "../src/plugin/GranularParameters.h"
#include <cmath>

using namespace GranularPlunderphonics;


TEST_CASE("Specific Parameter Tests", "[parameters]") {
    ParameterManager manager;
    GranularParameters::registerParameters(manager);

    SECTION("Grain Size Parameter") {
        auto param = manager.getParameter(kGrainSizeId);
        REQUIRE(param != nullptr);

        // Check range (1-100ms)
        REQUIRE(param->denormalize(0.0f) == 1.0f);  // min value
        REQUIRE(param->denormalize(1.0f) == 100.0f); // max value
        
        // Check default (50ms)
        float defaultNorm = param->getDefaultNormalizedValue();
        REQUIRE(param->denormalize(defaultNorm) == Catch::Approx(50.0f));

        // Check linearity
        float midPoint = param->denormalize(0.5f);
        REQUIRE(midPoint == Catch::Approx(50.5f)); // Linear interpolation

        // Test value conversion
        std::vector<float> testValues = {1.0f, 25.0f, 50.0f, 75.0f, 100.0f};
        for (float value : testValues) {
            float normalized = param->normalize(value);
            float denormalized = param->denormalize(normalized);
            REQUIRE(denormalized == Catch::Approx(value).margin(0.01f));
        }
    }

    SECTION("Grain Shape Parameter") {
        auto param = manager.getParameter(kGrainShapeId);
        REQUIRE(param != nullptr);

        // Check all enum values are accessible
        std::vector<GrainShapeType> shapes = {
            GrainShapeType::Sine,
            GrainShapeType::Triangle,
            GrainShapeType::Rectangle,
            GrainShapeType::Gaussian
        };

        for (GrainShapeType shape : shapes) {
            float normalized = param->normalize(static_cast<float>(shape));
            float denormalized = param->denormalize(normalized);
            REQUIRE(static_cast<int>(denormalized) == static_cast<int>(shape));
        }

        // Check default (Gaussian)
        float defaultNorm = param->getDefaultNormalizedValue();
        REQUIRE(static_cast<int>(param->denormalize(defaultNorm)) == 
                static_cast<int>(GrainShapeType::Gaussian));

        // Check string conversion
        REQUIRE(param->toString(param->normalize(static_cast<float>(GrainShapeType::Sine))) == "Sine");
        REQUIRE(param->toString(param->normalize(static_cast<float>(GrainShapeType::Triangle))) == "Triangle");
        REQUIRE(param->toString(param->normalize(static_cast<float>(GrainShapeType::Rectangle))) == "Rectangle");
        REQUIRE(param->toString(param->normalize(static_cast<float>(GrainShapeType::Gaussian))) == "Gaussian");
    }

    SECTION("Grain Density Parameter") {
        auto param = manager.getParameter(kGrainDensityId);
        REQUIRE(param != nullptr);

        // Check range (0.1-100Hz)
        REQUIRE(param->denormalize(0.0f) == Catch::Approx(0.1f));
        REQUIRE(param->denormalize(1.0f) == Catch::Approx(100.0f));

        // Check default (10Hz)
        float defaultNorm = param->getDefaultNormalizedValue();
        REQUIRE(param->denormalize(defaultNorm) == Catch::Approx(10.0f));

        // Verify logarithmic behavior
        float quarterPoint = param->denormalize(0.25f);
        float midPoint = param->denormalize(0.5f);
        float threeQuarterPoint = param->denormalize(0.75f);

        // In logarithmic mapping, should not be linear intervals
        REQUIRE(midPoint > (quarterPoint + threeQuarterPoint) / 2.0f);

        // Test value conversion at specific points
        std::vector<float> testValues = {0.1f, 1.0f, 10.0f, 50.0f, 100.0f};
        for (float value : testValues) {
            float normalized = param->normalize(value);
            float denormalized = param->denormalize(normalized);
            REQUIRE(denormalized == Catch::Approx(value).margin(value * 0.01f));
        }
    }

    SECTION("Parameter Smooth Changes") {
        auto sizeParam = manager.getParameter(kGrainSizeId);
        REQUIRE(sizeParam != nullptr);

        // Test smooth value changes
        const float sampleRate = 44100.0f;
        
        // Set initial and target values
        sizeParam->setNormalized(0.0f); // Start at 1ms
        sizeParam->setNormalized(1.0f); // Target 100ms

        // Should smoothly transition
        bool isSmooth = true;
        float lastValue = sizeParam->getSmoothedNormalized(sampleRate);

        for (int i = 0; i < 100; ++i) {
            float currentValue = sizeParam->getSmoothedNormalized(sampleRate);
            if (currentValue < lastValue) {
                isSmooth = false;
                break;
            }
            lastValue = currentValue;
        }

        REQUIRE(isSmooth);
    }

    SECTION("Parameter Thread Safety") {
        auto densityParam = manager.getParameter(kGrainDensityId);
        REQUIRE(densityParam != nullptr);

        constexpr int numIterations = 1000;
        std::atomic<bool> foundError{false};

        // Create threads for reading and writing
        std::thread writer([&]() {
            for (int i = 0; i < numIterations && !foundError; ++i) {
                float value = static_cast<float>(i) / numIterations;
                densityParam->setNormalized(value);
                std::this_thread::yield();
            }
        });

        std::thread reader([&]() {
            for (int i = 0; i < numIterations && !foundError; ++i) {
                float value = densityParam->getNormalized();
                if (value < 0.0f || value > 1.0f) {
                    foundError = true;
                }
                std::this_thread::yield();
            }
        });

        writer.join();
        reader.join();

        REQUIRE_FALSE(foundError);
    }
}
