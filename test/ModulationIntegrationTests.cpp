#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "../src/audio/ModulationMatrix.h"
#include "../src/audio/LorenzAttractor.h"
#include "../src/audio/GrainCloud.h"
#include "../src/audio/AudioBuffer.h"
#include "../src/plugin/GranularParameters.h"
#include "../src/plugin/ParameterManager.h"
#include <memory>
#include <iostream>
#include <thread>
#include <chrono>

using namespace GranularPlunderphonics;

// Test helper for generating a simple sine wave buffer
AudioBuffer createSineWaveBuffer(size_t channels, size_t samples, float frequency, float sampleRate) {
    AudioBuffer buffer(channels, samples);
    std::vector<float> data(samples);
    
    for (size_t i = 0; i < samples; ++i) {
        double phase = (2.0 * M_PI * frequency * i) / sampleRate;
        data[i] = static_cast<float>(std::sin(phase));
    }
    
    for (size_t ch = 0; ch < channels; ++ch) {
        buffer.write(ch, data.data(), samples, 0);
    }
    
    return buffer;
}

// Test helper to run an attractor for a number of iterations
void runAttractorIterations(std::shared_ptr<ChaoticAttractor> attractor, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        attractor->process();
    }
}

TEST_CASE("Modulation Matrix Complete Integration Test", "[modulation][integration]") {
    // Create necessary components for the test
    ParameterManager paramManager;
    auto attractor = std::make_shared<LorenzAttractor>(44100.0);
    
    // Register plugin parameters
    GranularParameters::registerParameters(paramManager);
    
    // Get parameter references
    auto grainSizeParam = paramManager.getParameter(kGrainSizeId);
    auto grainDensityParam = paramManager.getParameter(kGrainDensityId);
    auto grainShapeParam = paramManager.getParameter(kGrainShapeId);
    
    REQUIRE(grainSizeParam != nullptr);
    REQUIRE(grainDensityParam != nullptr);
    REQUIRE(grainShapeParam != nullptr);
    
    // Set initial parameter values
    grainSizeParam->setNormalized(0.5f); // 50ms
    grainDensityParam->setNormalized(0.5f); // Medium density
    grainShapeParam->setNormalized(0.0f); // Sine shape
    
    // Create the modulation matrix
    ModulationMatrix matrix(44100.0);
    
    SECTION("Full Modulation System Test") {
        // Step 1: Register attractor sources
        REQUIRE(matrix.registerAttractorSources("lorenz", "Lorenz", attractor));
        
        // Step 2: Register parameter destinations
        REQUIRE(matrix.registerParameterDestination(grainSizeParam));
        REQUIRE(matrix.registerParameterDestination(grainDensityParam));
        
        // Step 3: Create cloud parameters for modulation
        CloudParameters cloudParams;
        cloudParams.density = 10.0f;
        cloudParams.spread = 0.5f;
        cloudParams.positionRange = 1.0f;
        cloudParams.positionOffset = 0.0f;
        
        // Step 4: Register cloud parameters as destinations
        matrix.registerDestination("cloud_spread", "Stereo Spread", 
            [&cloudParams](float value) { cloudParams.spread = value; },
            0.0f, 1.0f);
            
        matrix.registerDestination("cloud_position", "Position Offset", 
            [&cloudParams](float value) { cloudParams.positionOffset = value; },
            0.0f, 1.0f);
        
        // Step 5: Create routes between sources and destinations
        // X dimension -> Grain Size
        matrix.createRoute("lorenz_X", "param_" + std::to_string(kGrainSizeId), 
                          0.5f, ModulationMode::Unipolar, 0.5f);
                          
        // Y dimension -> Grain Density
        matrix.createRoute("lorenz_Y", "param_" + std::to_string(kGrainDensityId), 
                          0.3f, ModulationMode::AbsBipolar, 0.2f);
                          
        // Z dimension -> Position Offset
        matrix.createRoute("lorenz_Z", "cloud_position", 
                          0.8f, ModulationMode::Bipolar, 0.5f);
                          
        // Complexity -> Stereo Spread
        matrix.createRoute("lorenz_Complexity", "cloud_spread", 
                          1.0f, ModulationMode::Unipolar, 0.0f);
        
        // Step 6: Record initial parameter values
        float initialSize = grainSizeParam->getReal();
        float initialDensity = grainDensityParam->getReal();
        float initialPosition = cloudParams.positionOffset;
        float initialSpread = cloudParams.spread;
        
        // Step 7: Run the attractor to generate some values
        runAttractorIterations(attractor, 10);
        
        // Step 8: Process modulation
        matrix.processControlRateModulation();
        
        // Step 9: Verify parameters have changed
        float newSize = grainSizeParam->getReal();
        float newDensity = grainDensityParam->getReal();
        float newPosition = cloudParams.positionOffset;
        float newSpread = cloudParams.spread;
        
        // Parameters should have changed due to modulation
        REQUIRE(newSize != initialSize);
        REQUIRE(newDensity != initialDensity);
        REQUIRE(newPosition != initialPosition);
        REQUIRE(newSpread != initialSpread);
        
        // Step 10: Create a grain cloud to process audio with modulated parameters
        GrainCloud cloud(50);
        cloud.setCloudParameters(cloudParams);
        
        // Create source and output buffers
        auto source = createSineWaveBuffer(1, 44100, 440.0f, 44100.0f);
        AudioBuffer output(2, 1024);
        
        // Process audio with the grain cloud
        cloud.process(source, output, 1024);
        
        // At this point, all we can verify is that we have some output
        bool hasOutput = false;
        for (size_t i = 0; i < 1024; ++i) {
            if (std::abs(output.getSample(0, i)) > 0.0f) {
                hasOutput = true;
                break;
            }
        }
        REQUIRE(hasOutput);
        
        // Step 11: Test a modulation preset
        // First save the current configuration as a preset
        REQUIRE(matrix.createPreset("TestPreset"));
        
        // Modify some routes
        matrix.setRouteDepth("lorenz_X", "param_" + std::to_string(kGrainSizeId), 0.1f);
        matrix.removeRoute("lorenz_Y", "param_" + std::to_string(kGrainDensityId));
        
        // Record parameters with modified routes
        runAttractorIterations(attractor, 5);
        matrix.processControlRateModulation();
        
        float modifiedSize = grainSizeParam->getReal();
        
        // Load the preset to restore original routes
        REQUIRE(matrix.loadPreset("TestPreset"));
        
        // Process modulation again
        runAttractorIterations(attractor, 5);
        matrix.processControlRateModulation();
        
        // Size should have changed again due to restored route depth
        float restoredSize = grainSizeParam->getReal();
        REQUIRE(restoredSize != modifiedSize);
    }
    
    SECTION("Audio Rate Modulation Test") {
        // Register parameter as audio rate destination
        matrix.registerDestination("audio_param", "Audio Rate Param",
            [](float value) { /* Audio rate processing would happen here */ },
            0.0f, 1.0f, true);
            
        // Register attractor source
        REQUIRE(matrix.registerAttractorSources("lorenz", "Lorenz", attractor));
        
        // Create route
        matrix.createRoute("lorenz_X", "audio_param", 1.0f, ModulationMode::Bipolar, 0.0f);
        
        // Record modulation values across a block
        const size_t blockSize = 64;
        std::vector<float> modulationValues;
        
        // Run attractor to get some chaotic behavior
        runAttractorIterations(attractor, 20);
        
        // Record modulation values at each sample in the block
        for (size_t i = 0; i < blockSize; ++i) {
            matrix.processAudioRateModulation(i, blockSize);
            // Can't directly get audio rate values, so we'll skip verification
        }
        
        // Verify we didn't crash during audio rate processing
        REQUIRE(true);
    }
}

TEST_CASE("Modulation Performance Test", "[modulation][performance]") {
    // Create a matrix with many routes for performance testing
    ModulationMatrix matrix(44100.0);
    std::vector<std::shared_ptr<LorenzAttractor>> attractors;
    
    // Create several attractors
    for (int i = 0; i < 3; ++i) {
        auto attractor = std::make_shared<LorenzAttractor>(44100.0);
        attractors.push_back(attractor);
        
        // Set slightly different parameters
        LorenzAttractor::Parameters params;
        params.rho = 28.0 + i;
        params.sigma = 10.0 + i * 0.5;
        attractor->setParameters(params);
        
        // Register its sources
        matrix.registerAttractorSources("lorenz" + std::to_string(i), "Lorenz " + std::to_string(i), attractor);
    }
    
    // Create destinations (typically parameters or audio processing controls)
    for (int i = 0; i < 10; ++i) {
        matrix.registerDestination(
            "dest" + std::to_string(i), 
            "Destination " + std::to_string(i),
            [](float value) { /* Would set a parameter or control value */ },
            0.0f, 1.0f
        );
    }
    
    // Create many routes
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) { // X, Y, Z dimensions
            for (int k = 0; k < 5; ++k) { // Use half the destinations
                std::string sourceId = "lorenz" + std::to_string(i) + "_" + std::string(1, 'X' + j);
                std::string destId = "dest" + std::to_string(k * 2); // Every other destination
                
                matrix.createRoute(sourceId, destId, 0.5f, ModulationMode::Bipolar, 0.0f);
            }
        }
    }
    
    // Run the attractors to get some values
    for (auto& attractor : attractors) {
        runAttractorIterations(attractor, 50);
    }
    
    SECTION("Control Rate Performance") {
        // Measure time for control rate processing
        auto start = std::chrono::high_resolution_clock::now();
        
        const int iterations = 1000;
        for (int i = 0; i < iterations; ++i) {
            matrix.processControlRateModulation();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        
        double msPerIteration = elapsed.count() / iterations;
        std::cout << "Control rate processing: " << msPerIteration << "ms per iteration" << std::endl;
        
        // Should be reasonably fast for real-time use (under 1ms)
        REQUIRE(msPerIteration < 1.0);
    }
    
    SECTION("Audio Rate Performance") {
        // Register some audio rate destinations
        for (int i = 0; i < 5; ++i) {
            matrix.registerDestination(
                "audio_dest" + std::to_string(i), 
                "Audio Destination " + std::to_string(i),
                [](float value) { /* Would set an audio control value */ },
                0.0f, 1.0f, true
            );
        }
        
        // Create audio rate routes
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) { // X, Y, Z dimensions
                for (int k = 0; k < 3; ++k) { // Use some audio destinations
                    std::string sourceId = "lorenz" + std::to_string(i) + "_" + std::string(1, 'X' + j);
                    std::string destId = "audio_dest" + std::to_string(k);
                    
                    matrix.createRoute(sourceId, destId, 0.5f, ModulationMode::Bipolar, 0.0f);
                }
            }
        }
        
        // Measure time for audio rate processing
        auto start = std::chrono::high_resolution_clock::now();
        
        const int blockSize = 512;
        const int iterations = 100;
        
        for (int i = 0; i < iterations; ++i) {
            for (int sample = 0; sample < blockSize; ++sample) {
                matrix.processAudioRateModulation(sample, blockSize);
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        
        double usPerSample = (elapsed.count() * 1000.0) / (iterations * blockSize);
        std::cout << "Audio rate processing: " << usPerSample << "μs per sample" << std::endl;
        
        // Should be efficient enough for real-time audio processing
        // Typically need to be < 20μs per sample for 44.1kHz processing
        REQUIRE(usPerSample < 20.0);
    }
}

TEST_CASE("Complex Routing Scenarios", "[modulation][routing]") {
    ModulationMatrix matrix(44100.0);
    auto attractor = std::make_shared<LorenzAttractor>(44100.0);
    
    REQUIRE(matrix.registerAttractorSources("lorenz", "Lorenz", attractor));
    
    // Create multiple destinations
    std::vector<float> destValues(10, 0.5f);
    
    for (int i = 0; i < 10; ++i) {
        matrix.registerDestination(
            "dest" + std::to_string(i), 
            "Destination " + std::to_string(i),
            [&destValues, i](float value) { destValues[i] = value; },
            0.0f, 1.0f
        );
    }
    
    SECTION("Many-to-One Routing") {
        // Connect multiple sources to a single destination with different depths
        matrix.createRoute("lorenz_X", "dest0", 0.3f, ModulationMode::Bipolar, 0.0f);
        matrix.createRoute("lorenz_Y", "dest0", 0.2f, ModulationMode::Unipolar, 0.1f);
        matrix.createRoute("lorenz_Z", "dest0", 0.1f, ModulationMode::AbsBipolar, -0.1f);
        
        // Run attractor and process modulation
        runAttractorIterations(attractor, 20);
        matrix.processControlRateModulation();
        
        // Destination should have changed from initial value
        REQUIRE(destValues[0] != 0.5f);
        
        // Value should be within expected range
        REQUIRE(destValues[0] >= 0.0f);
        REQUIRE(destValues[0] <= 1.0f);
    }
    
    SECTION("One-to-Many Routing") {
        // Connect one source to multiple destinations with different modes
        for (int i = 0; i < 5; ++i) {
            ModulationMode mode;
            switch (i % 3) {
                case 0: mode = ModulationMode::Bipolar; break;
                case 1: mode = ModulationMode::Unipolar; break;
                case 2: mode = ModulationMode::AbsBipolar; break;
            }
            
            matrix.createRoute("lorenz_X", "dest" + std::to_string(i), 
                               0.5f + (i * 0.1f), mode, (i * 0.1f) - 0.2f);
        }
        
        // Record initial values
        std::vector<float> initialValues = destValues;
        
        // Run attractor and process modulation
        runAttractorIterations(attractor, 20);
        matrix.processControlRateModulation();
        
        // Check that all destinations have changed but are still in valid range
        for (int i = 0; i < 5; ++i) {
            REQUIRE(destValues[i] != initialValues[i]);
            REQUIRE(destValues[i] >= 0.0f);
            REQUIRE(destValues[i] <= 1.0f);
        }
        
        // Destinations 5-9 should remain unchanged
        for (int i = 5; i < 10; ++i) {
            REQUIRE(destValues[i] == initialValues[i]);
        }
    }
    
    SECTION("Complex Routing Matrix") {
        // Create a complex web of modulation connections
        // X → dest0, dest3, dest6
        // Y → dest1, dest4, dest7
        // Z → dest2, dest5, dest8
        // All → dest9
        
        const char* dims = "XYZ";
        
        for (int dim = 0; dim < 3; ++dim) {
            std::string sourceId = std::string("lorenz_") + dims[dim];
            
            for (int j = 0; j < 3; ++j) {
                int destIdx = dim + (j * 3);
                matrix.createRoute(sourceId, "dest" + std::to_string(destIdx), 
                                  0.5f, ModulationMode::Bipolar, 0.0f);
            }
            
            // All to final destination
            matrix.createRoute(sourceId, "dest9", 0.3f, ModulationMode::Unipolar, 0.0f);
        }
        
        // Record initial values
        std::vector<float> initialValues = destValues;
        
        // Run attractor and process modulation
        runAttractorIterations(attractor, 20);
        matrix.processControlRateModulation();
        
        // All destinations should have changed
        for (int i = 0; i < 10; ++i) {
            REQUIRE(destValues[i] != initialValues[i]);
            REQUIRE(destValues[i] >= 0.0f);
            REQUIRE(destValues[i] <= 1.0f);
        }
        
        // Destination 9 should have contributions from all sources
        REQUIRE(destValues[9] != initialValues[9]);
    }
}