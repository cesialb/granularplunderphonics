/**
 * @file ParameterTests.cpp
 * @brief Unit tests for the parameter management system
 */

#include <iostream>
#include <vector>
#include <memory>
#include <cmath>
#include <thread>
#include <atomic>
#include <sstream>
#include <chrono>
#include "../src/plugin/ParameterManager.h"
#include "../src/plugin/GranularParameters.h"
#include "../src/common/Logger.h"

namespace GranularPlunderphonics {
namespace Tests {

// Simple assertion helper
void checkCondition(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

// Test parameter registration
void testParameterRegistration() {
    Logger logger("ParameterTest");
    logger.info("Testing parameter registration");

    ParameterManager manager;

    // Register parameters
    bool success = GranularParameters::registerParameters(manager);
    checkCondition(success, "Parameter registration failed");

    // Check parameter count
    size_t expectedCount = 4; // Bypass + 3 granular parameters
    checkCondition(manager.getParameterCount() == expectedCount,
                  "Incorrect parameter count after registration");

    // Check specific parameters
    auto grainSize = manager.getParameter(kGrainSizeId);
    checkCondition(grainSize != nullptr, "Grain size parameter not found");
    checkCondition(grainSize->getName() == "Grain Size", "Incorrect parameter name");

    auto grainShape = manager.getParameter(kGrainShapeId);
    checkCondition(grainShape != nullptr, "Grain shape parameter not found");

    auto grainDensity = manager.getParameter(kGrainDensityId);
    checkCondition(grainDensity != nullptr, "Grain density parameter not found");

    logger.info("Parameter registration test completed successfully");
}

// Test parameter normalization/denormalization
void testParameterNormalization() {
    Logger logger("ParameterTest");
    logger.info("Testing parameter normalization/denormalization");

    // Test float parameter normalization (linear)
    auto sizeParam = GranularParameters::createGrainSize();

    // Test denormalization
    checkCondition(std::abs(sizeParam->denormalize(0.0f) - 1.0f) < 0.001f,
                  "Incorrect min value after denormalization");
    checkCondition(std::abs(sizeParam->denormalize(1.0f) - 100.0f) < 0.001f,
                  "Incorrect max value after denormalization");
    checkCondition(std::abs(sizeParam->denormalize(0.5f) - 50.5f) < 0.001f,
                  "Incorrect mid value after denormalization");

    // Test normalization
    checkCondition(std::abs(sizeParam->normalize(1.0f) - 0.0f) < 0.001f,
                  "Incorrect normalization for min value");
    checkCondition(std::abs(sizeParam->normalize(100.0f) - 1.0f) < 0.001f,
                  "Incorrect normalization for max value");
    checkCondition(std::abs(sizeParam->normalize(50.5f) - 0.5f) < 0.001f,
                  "Incorrect normalization for mid value");

    // Test float parameter normalization (logarithmic)
    auto densityParam = GranularParameters::createGrainDensity();

    // Verify logarithmic behavior
    float quarter = densityParam->denormalize(0.25f);
    float half = densityParam->denormalize(0.5f);
    float threeQuarters = densityParam->denormalize(0.75f);

    // In logarithmic mapping, these points should not be linearly distributed
    checkCondition(half > (quarter + threeQuarters) / 2.0f,
                  "Logarithmic mapping not working correctly");

    // Test normalization/denormalization consistency
    float testValues[] = {0.1f, 1.0f, 10.0f, 50.0f, 100.0f};
    for (float value : testValues) {
        float normalized = densityParam->normalize(value);
        float denormalized = densityParam->denormalize(normalized);
        checkCondition(std::abs(value - denormalized) < 0.001f * value,
                      "Normalization/denormalization cycle not consistent");
    }

    // Test enum parameter
    auto shapeParam = GranularParameters::createGrainShape();

    checkCondition(static_cast<int>(shapeParam->denormalize(0.0f)) ==
                  static_cast<int>(GrainShapeType::Sine),
                  "Incorrect enum value at index 0");

    checkCondition(static_cast<int>(shapeParam->denormalize(1.0f)) ==
                  static_cast<int>(GrainShapeType::Gaussian),
                  "Incorrect enum value at index 3");

    // Test bool parameter
    auto bypassParam = GranularParameters::createBypass();

    checkCondition(bypassParam->denormalize(0.0f) < 0.5f, "Boolean false value incorrect");
    checkCondition(bypassParam->denormalize(1.0f) > 0.5f, "Boolean true value incorrect");
    checkCondition(bypassParam->denormalize(0.49f) < 0.5f, "Boolean threshold incorrect");
    checkCondition(bypassParam->denormalize(0.51f) > 0.5f, "Boolean threshold incorrect");

    logger.info("Parameter normalization test completed successfully");
}

// Test smooth parameter changes
void testSmoothParameterChanges() {
    Logger logger("ParameterTest");
    logger.info("Testing smooth parameter changes");

    // Create parameter with known smoothing time
    const float smoothingTimeMs = 100.0f;
    auto sizeParam = std::make_shared<FloatParameter>(
        kGrainSizeId,
        "Test Param",
        "Test",
        0.0f,
        100.0f,
        50.0f,
        ParameterFlags::NoFlags,
        "",
        smoothingTimeMs
    );

    // Set initial value
    sizeParam->setReal(0.0f);

    // Set target value
    sizeParam->setReal(100.0f);

    // Calculate how many steps we need based on sample rate
    const float sampleRate = 44100.0f;
    const float smoothingSamples = (smoothingTimeMs / 1000.0f) * sampleRate;
    const int numSteps = static_cast<int>(smoothingSamples * 1.5f); // 1.5x to ensure completion

    // Check that value changes smoothly
    float lastValue = sizeParam->getSmoothedReal(sampleRate);

    bool smoothlyIncreasing = true;
    float previousValue = lastValue;

    for (int i = 0; i < numSteps; ++i) {
        float currentValue = sizeParam->getSmoothedReal(sampleRate);

        // Ensure value is monotonically increasing
        if (currentValue < previousValue) {
            smoothlyIncreasing = false;
            break;
        }

        previousValue = currentValue;
    }

    checkCondition(smoothlyIncreasing, "Parameter value not increasing smoothly");

    // Check that final value matches target
    float finalValue = sizeParam->getSmoothedReal(sampleRate);
    checkCondition(std::abs(finalValue - 100.0f) < 0.01f,
                  "Final smoothed value does not match target");

    logger.info("Smooth parameter change test completed successfully");
}

// Test parameter persistence
void testParameterPersistence() {
    Logger logger("ParameterTest");
    logger.info("Testing parameter persistence");

    // Create parameter manager and register parameters
    ParameterManager manager1;
    GranularParameters::registerParameters(manager1);

    // Modify parameter values
    manager1.setParameterNormalized(kGrainSizeId, 0.75f);    // 75% of range
    manager1.setParameterNormalized(kGrainShapeId, 0.33f);   // ~1st option
    manager1.setParameterNormalized(kGrainDensityId, 0.8f);  // 80% on log scale
    manager1.setParameterNormalized(kBypassId, 1.0f);        // true

    // Save state to stream
    std::stringstream stateStream;
    bool saveSuccess = manager1.saveState(stateStream);
    checkCondition(saveSuccess, "Failed to save parameter state");

    // Create a new manager and load the state
    ParameterManager manager2;
    GranularParameters::registerParameters(manager2);

    // Pre-load check - values should be at defaults
    float defaultGrainSize = manager2.getParameterNormalized(kGrainSizeId);
    checkCondition(std::abs(defaultGrainSize - 0.5f) < 0.01f,
                  "Default grain size parameter not at expected value");

    // Load the state
    bool loadSuccess = manager2.loadState(stateStream);
    checkCondition(loadSuccess, "Failed to load parameter state");

    // Verify loaded values match original values
    float grainSizeValue = manager2.getParameterNormalized(kGrainSizeId);
    float grainShapeValue = manager2.getParameterNormalized(kGrainShapeId);
    float grainDensityValue = manager2.getParameterNormalized(kGrainDensityId);
    float bypassValue = manager2.getParameterNormalized(kBypassId);

    checkCondition(std::abs(grainSizeValue - 0.75f) < 0.001f,
                  "Loaded grain size value does not match saved value");
    checkCondition(std::abs(grainShapeValue - 0.33f) < 0.001f,
                  "Loaded grain shape value does not match saved value");
    checkCondition(std::abs(grainDensityValue - 0.8f) < 0.001f,
                  "Loaded grain density value does not match saved value");
    checkCondition(std::abs(bypassValue - 1.0f) < 0.001f,
                  "Loaded bypass value does not match saved value");

    logger.info("Parameter persistence test completed successfully");
}

// Test parameter thread safety
void testParameterThreadSafety() {
    Logger logger("ParameterTest");
    logger.info("Testing parameter thread safety");

    ParameterManager manager;
    GranularParameters::registerParameters(manager);

    // Number of iterations for each thread
    const int iterations = 10000;

    // Atomic flag to synchronize thread start
    std::atomic<bool> startFlag(false);

    // Atomic counter for synchronization
    std::atomic<int> threadsReady(0);
    std::atomic<int> errorCount(0);

    // Thread function for UI thread simulation
    auto uiThreadFunc = [&](int threadId) {
        threadsReady++;

        // Wait for all threads to be ready
        while (!startFlag.load()) {
            std::this_thread::yield();
        }

        try {
            // Simulate UI thread setting parameters
            for (int i = 0; i < iterations; ++i) {
                float value = static_cast<float>(i % 100) / 100.0f;

                // Alternate between parameters based on thread ID
                ParamID paramId;
                switch (threadId % 3) {
                    case 0: paramId = kGrainSizeId; break;
                    case 1: paramId = kGrainShapeId; break;
                    case 2: paramId = kGrainDensityId; break;
                }

                manager.setParameterNormalized(paramId, value);
            }
        } catch (const std::exception& e) {
            errorCount++;
            logger.error("UI thread %d error: %s", threadId, e.what());
        }
    };

    // Thread function for audio thread simulation
    auto audioThreadFunc = [&]() {
        threadsReady++;

        // Wait for all threads to be ready
        while (!startFlag.load()) {
            std::this_thread::yield();
        }

        try {
            // Simulate audio thread reading parameters
            const float sampleRate = 44100.0f;
            for (int i = 0; i < iterations; ++i) {
                // Process parameter changes (smoothing)
                manager.processParameterChanges(sampleRate);

                // Read all parameters
                auto grainSize = manager.getParameter(kGrainSizeId);
                auto grainShape = manager.getParameter(kGrainShapeId);
                auto grainDensity = manager.getParameter(kGrainDensityId);

                if (grainSize) {
                    float value = grainSize->getSmoothedReal(sampleRate);
                    // Just using the value to ensure it's not optimized away
                    if (value < 0.0f || value > 100.0f) {
                        throw std::runtime_error("Invalid grain size value");
                    }
                }

                if (grainShape) {
                    float value = grainShape->getSmoothedReal(sampleRate);
                    if (value < 0.0f || value > 3.0f) {
                        throw std::runtime_error("Invalid grain shape value");
                    }
                }

                if (grainDensity) {
                    float value = grainDensity->getSmoothedReal(sampleRate);
                    if (value < 0.1f || value > 100.0f) {
                        throw std::runtime_error("Invalid grain density value");
                    }
                }
            }
        } catch (const std::exception& e) {
            errorCount++;
            logger.error("Audio thread error: %s", e.what());
        }
    };

    // Create threads
    std::vector<std::thread> threads;
    const int numUiThreads = 3;

    // Create UI threads
    for (int i = 0; i < numUiThreads; ++i) {
        threads.emplace_back(uiThreadFunc, i);
    }

    // Create audio thread
    threads.emplace_back(audioThreadFunc);

    // Wait for all threads to be ready
    while (threadsReady.load() < numUiThreads + 1) {
        std::this_thread::yield();
    }

    // Start all threads simultaneously
    startFlag.store(true);

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Check for errors
    checkCondition(errorCount.load() == 0,
                  "Thread safety test encountered errors");

    logger.info("Thread safety test completed successfully");
}

// Function to register all parameter tests
void registerParameterTests() {
    // Register all parameter tests
    extern void registerTest(const std::string& name, std::function<void()> testFunc);

    registerTest("Parameter_Registration", testParameterRegistration);
    registerTest("Parameter_Normalization", testParameterNormalization);
    registerTest("Parameter_SmoothChanges", testSmoothParameterChanges);
    registerTest("Parameter_Persistence", testParameterPersistence);
    registerTest("Parameter_ThreadSafety", testParameterThreadSafety);
}

} // namespace Tests
} // namespace GranularPlunderphonics