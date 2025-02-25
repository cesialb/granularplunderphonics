#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "../src/audio/ModulationMatrix.h"
#include "../src/audio/LorenzAttractor.h"
#include <memory>
#include <vector>
#include <atomic>
#include <thread>
#include <chrono>

using namespace GranularPlunderphonics;

namespace {
    // Simple mock source that provides controlled values
    class MockModulationSource {
    public:
        explicit MockModulationSource(float initialValue = 0.0f)
            : mValue(initialValue) {}
        
        float getValue() const { return mValue.load(); }
        void setValue(float value) { mValue.store(value); }
        
    private:
        std::atomic<float> mValue;
    };
    
    // Simple mock destination that stores received values
    class MockModulationDestination {
    public:
        explicit MockModulationDestination() 
            : mValue(0.0f), mUpdatedCount(0) {}
        
        void setValue(float value) { 
            mValue.store(value); 
            mUpdatedCount++;
        }
        
        float getValue() const { return mValue.load(); }
        int getUpdatedCount() const { return mUpdatedCount.load(); }
        
    private:
        std::atomic<float> mValue;
        std::atomic<int> mUpdatedCount;
    };
}

TEST_CASE("Modulation Matrix Basic Tests", "[modulation]") {
    ModulationMatrix matrix(44100.0);
    
    SECTION("Source Registration") {
        // Create mock source
        auto mockSource = std::make_shared<MockModulationSource>(0.5f);
        
        // Register source
        bool result = matrix.registerSource(
            "test_source", "Test Source",
            [mockSource]() { return mockSource->getValue(); },
            true, -1.0f, 1.0f
        );
        
        REQUIRE(result);
        
        // Verify source was registered
        auto sources = matrix.getAllSources();
        REQUIRE(sources.size() == 1);
        REQUIRE(sources[0].id == "test_source");
        REQUIRE(sources[0].name == "Test Source");
        
        // Verify the source getter works
        mockSource->setValue(0.75f);
        REQUIRE(sources[0].valueGetter() == 0.75f);
    }
    
    SECTION("Destination Registration") {
        // Create mock destination
        auto mockDest = std::make_shared<MockModulationDestination>();
        
        // Register destination
        bool result = matrix.registerDestination(
            "test_dest", "Test Destination",
            [mockDest](float value) { mockDest->setValue(value); },
            0.0f, 1.0f, false
        );
        
        REQUIRE(result);
        
        // Verify destination was registered
        auto destinations = matrix.getAllDestinations();
        REQUIRE(destinations.size() == 1);
        REQUIRE(destinations[0].id == "test_dest");
        REQUIRE(destinations[0].name == "Test Destination");
        
        // Verify the destination setter works
        destinations[0].valueSetter(0.6f);
        REQUIRE(mockDest->getValue() == 0.6f);
    }
    
    SECTION("Route Creation") {
        // Create and register source and destination
        auto mockSource = std::make_shared<MockModulationSource>(0.5f);
        auto mockDest = std::make_shared<MockModulationDestination>();
        
        matrix.registerSource(
            "test_source", "Test Source",
            [mockSource]() { return mockSource->getValue(); }
        );
        
        matrix.registerDestination(
            "test_dest", "Test Destination",
            [mockDest](float value) { mockDest->setValue(value); }
        );
        
        // Create route
        std::string routeId = matrix.createRoute(
            "test_source", "test_dest", 1.0f, ModulationMode::Bipolar, 0.0f
        );
        
        REQUIRE(!routeId.empty());
        REQUIRE(matrix.routeExists("test_source", "test_dest"));
        
        // Verify route was created
        auto routes = matrix.getAllRoutes();
        REQUIRE(routes.size() == 1);
        REQUIRE(routes[0].sourceId == "test_source");
        REQUIRE(routes[0].destinationId == "test_dest");
        REQUIRE(routes[0].depth == 1.0f);
    }
}

TEST_CASE("Modulation Routing and Processing", "[modulation]") {
    ModulationMatrix matrix(44100.0);
    
    // Create test components
    auto source1 = std::make_shared<MockModulationSource>(0.0f);
    auto source2 = std::make_shared<MockModulationSource>(0.0f);
    auto destination = std::make_shared<MockModulationDestination>();
    
    // Register components
    matrix.registerSource("source1", "Source 1", [source1]() { return source1->getValue(); });
    matrix.registerSource("source2", "Source 2", [source2]() { return source2->getValue(); });
    matrix.registerDestination("dest", "Destination", 
                             [destination](float value) { destination->setValue(value); });
    
    SECTION("Simple One-to-One Modulation") {
        // Create route
        matrix.createRoute("source1", "dest", 1.0f, ModulationMode::Bipolar, 0.0f);
        
        // Set source value
        source1->setValue(0.5f);
        
        // Process modulation
        matrix.processControlRateModulation();
        
        // Check destination value
        REQUIRE(destination->getValue() == Catch::Approx(0.5f));
        
        // Change source value
        source1->setValue(-0.3f);
        matrix.processControlRateModulation();
        
        // Check destination updated
        REQUIRE(destination->getValue() == Catch::Approx(-0.3f));
    }
    
    SECTION("Many-to-One Modulation") {
        // Create routes
        matrix.createRoute("source1", "dest", 0.5f, ModulationMode::Bipolar, 0.0f);
        matrix.createRoute("source2", "dest", 0.5f, ModulationMode::Bipolar, 0.0f);
        
        // Set source values
        source1->setValue(0.4f);
        source2->setValue(0.2f);
        
        // Process modulation
        matrix.processControlRateModulation();
        
        // Check destination value (sum of weighted sources)
        // 0.4 * 0.5 + 0.2 * 0.5 = 0.3
        REQUIRE(destination->getValue() == Catch::Approx(0.3f));
    }
    
    SECTION("Modulation Mode Changes") {
        // Create route with bipolar mode
        matrix.createRoute("source1", "dest", 1.0f, ModulationMode::Bipolar, 0.0f);
        
        // Test bipolar mode
        source1->setValue(-0.5f);
        matrix.processControlRateModulation();
        REQUIRE(destination->getValue() == Catch::Approx(-0.5f));
        
        // Change to unipolar mode
        matrix.setRouteMode("source1", "dest", ModulationMode::Unipolar);
        matrix.processControlRateModulation();
        
        // -0.5 in bipolar becomes 0.25 in unipolar
        REQUIRE(destination->getValue() == Catch::Approx(0.25f));
        
        // Change to absolute mode
        matrix.setRouteMode("source1", "dest", ModulationMode::AbsBipolar);
        matrix.processControlRateModulation();
        
        // |-0.5| = 0.5
        REQUIRE(destination->getValue() == Catch::Approx(0.5f));
    }
    
    SECTION("Depth Control") {
        // Create route with half depth
        matrix.createRoute("source1", "dest", 0.5f, ModulationMode::Bipolar, 0.0f);
        
        // Set source value
        source1->setValue(1.0f);
        matrix.processControlRateModulation();
        
        // Check destination value is scaled by depth
        REQUIRE(destination->getValue() == Catch::Approx(0.5f));
        
        // Change depth
        matrix.setRouteDepth("source1", "dest", 0.25f);
        matrix.processControlRateModulation();
        
        // Check updated value
        REQUIRE(destination->getValue() == Catch::Approx(0.25f));
    }
    
    SECTION("Offset Control") {
        // Create route with offset
        matrix.createRoute("source1", "dest", 1.0f, ModulationMode::Bipolar, 0.5f);
        
        // Set source value
        source1->setValue(0.0f);
        matrix.processControlRateModulation();
        
        // Check only offset is applied
        REQUIRE(destination->getValue() == Catch::Approx(0.5f));
        
        // Set source to negative
        source1->setValue(-0.2f);
        matrix.processControlRateModulation();
        
        // Check value is source + offset
        REQUIRE(destination->getValue() == Catch::Approx(0.3f));
    }
}

TEST_CASE("Attractor Integration Tests", "[modulation]") {
    ModulationMatrix matrix(44100.0);
    
    // Create Lorenz attractor
    auto attractor = std::make_shared<LorenzAttractor>(44100.0);
    
    // Create test destination
    auto destination = std::make_shared<MockModulationDestination>();
    
    // Register components
    REQUIRE(matrix.registerAttractorSources("lorenz", "Lorenz", attractor));
    
    matrix.registerDestination("dest", "Destination", 
                             [destination](float value) { destination->setValue(value); });
    
    SECTION("Attractor Source Registration") {
        // Check that all dimensions were registered
        auto sources = matrix.getAllSources();
        
        // Should have X, Y, Z dimensions plus periodicity and complexity
        REQUIRE(sources.size() == 5);
        
        // Verify source IDs
        bool hasSourceX = false;
        bool hasSourceY = false;
        bool hasSourceZ = false;
        
        for (const auto& source : sources) {
            if (source.id == "lorenz_X") hasSourceX = true;
            if (source.id == "lorenz_Y") hasSourceY = true;
            if (source.id == "lorenz_Z") hasSourceZ = true;
        }
        
        REQUIRE(hasSourceX);
        REQUIRE(hasSourceY);
        REQUIRE(hasSourceZ);
    }
    
    SECTION("Attractor Modulation") {
        // Create route from X dimension
        matrix.createRoute("lorenz_X", "dest", 1.0f, ModulationMode::Bipolar, 0.0f);
        
        // Process the attractor to generate some values
        attractor->process();
        
        // Process modulation
        matrix.processControlRateModulation();
        
        // Check destination received a value
        // Note: We can't predict exact value, but it should be within range
        float value = destination->getValue();
        REQUIRE(value >= -1.0f);
        REQUIRE(value <= 1.0f);
        
        // Process more steps and check for changes
        for (int i = 0; i < 10; i++) {
            attractor->process();
        }
        
        matrix.processControlRateModulation();
        float newValue = destination->getValue();
        
        // Value should have changed (Lorenz is chaotic)
        REQUIRE(newValue != value);
    }
}

TEST_CASE("Modulation Smoothing Tests", "[modulation]") {
    ModulationMatrix matrix(44100.0);
    
    // Create test components
    auto source = std::make_shared<MockModulationSource>(0.0f);
    auto destination = std::make_shared<MockModulationDestination>();
    
    // Register components
    matrix.registerSource("source", "Source", [source]() { return source->getValue(); });
    matrix.registerDestination("dest", "Destination", 
                             [destination](float value) { destination->setValue(value); });
    
    // Create route with smoothing
    matrix.createRoute("source", "dest", 1.0f, ModulationMode::Bipolar, 0.0f);
    matrix.setRouteSmoothingTime("source", "dest", 100.0f); // 100ms smoothing
    
    SECTION("Control Rate Smoothing") {
        // Set source to initial value
        source->setValue(0.0f);
        matrix.processControlRateModulation();
        
        // Make a sudden change
        source->setValue(1.0f);
        
        // Process a few times (simulating multiple processing blocks)
        matrix.processControlRateModulation(); // First block
        float value1 = destination->getValue();
        
        matrix.processControlRateModulation(); // Second block
        float value2 = destination->getValue();
        
        matrix.processControlRateModulation(); // Third block
        float value3 = destination->getValue();
        
        // Values should gradually increase toward 1.0
        REQUIRE(value1 > 0.0f);
        REQUIRE(value2 > value1);
        REQUIRE(value3 > value2);
        REQUIRE(value3 < 1.0f); // Still not reached target
    }
    
    SECTION("Smoothing Reset") {
        // Set source to initial value
        source->setValue(0.0f);
        matrix.processControlRateModulation();
        
        // Make a sudden change
        source->setValue(1.0f);
        
        // Process once to start smoothing
        matrix.processControlRateModulation();
        float partialValue = destination->getValue();
        REQUIRE(partialValue > 0.0f);
        REQUIRE(partialValue < 1.0f);
        
        // Reset smoothing
        matrix.resetSmoothing();
        
        // Value should jump immediately to target
        matrix.processControlRateModulation();
        REQUIRE(destination->getValue() == Catch::Approx(1.0f));
    }
    
    SECTION("Audio Rate Smoothing") {
        // Register an audio-rate destination
        auto audioDestination = std::make_shared<MockModulationDestination>();
        matrix.registerDestination("audio_dest", "Audio Destination", 
                                  [audioDestination](float value) { audioDestination->setValue(value); },
                                  0.0f, 1.0f, true);
        
        // Create audio-rate route
        matrix.createRoute("source", "audio_dest", 1.0f, ModulationMode::Bipolar, 0.0f);
        
        // Set source to initial value
        source->setValue(0.0f);
        
        // Make a sudden change
        source->setValue(1.0f);
        
        // Process at audio rate (simulate processing a block of samples)
        const size_t blockSize = 64;
        std::vector<float> values;
        
        for (size_t i = 0; i < blockSize; ++i) {
            matrix.processAudioRateModulation(i, blockSize);
            values.push_back(audioDestination->getValue());
        }
        
        // Values should increase continuously
        for (size_t i = 1; i < values.size(); ++i) {
            REQUIRE(values[i] >= values[i-1]);
        }
        
        // First value should be > 0 (started smooth transition)
        REQUIRE(values.front() > 0.0f);
        
        // Last value should be < 1.0 (still smoothing)
        REQUIRE(values.back() < 1.0f);
    }
}

TEST_CASE("Modulation Matrix Thread Safety", "[modulation]") {
    ModulationMatrix matrix(44100.0);
    
    // Create test components
    auto source = std::make_shared<MockModulationSource>(0.0f);
    auto destination = std::make_shared<MockModulationDestination>();
    
    // Register components
    matrix.registerSource("source", "Source", [source]() { return source->getValue(); });
    matrix.registerDestination("dest", "Destination", 
                             [destination](float value) { destination->setValue(value); });
    
    // Create route
    matrix.createRoute("source", "dest", 1.0f, ModulationMode::Bipolar, 0.0f);
    
    SECTION("Concurrent Processing and Modification") {
        std::atomic<bool> keepRunning{true};
        std::atomic<bool> encounteredError{false};
        
        // Thread for processing
        std::thread processingThread([&]() {
            try {
                while (keepRunning) {
                    matrix.processControlRateModulation();
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            } catch (...) {
                encounteredError = true;
            }
        });
        
        // Thread for modifying parameters
        std::thread paramThread([&]() {
            try {
                for (int i = 0; i < 100 && keepRunning; ++i) {
                    // Change source value
                    source->setValue(static_cast<float>(i % 10) / 10.0f);
                    
                    // Modify route parameters
                    matrix.setRouteDepth("source", "dest", static_cast<float>(i % 10) / 10.0f);
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    
                    matrix.setRouteOffset("source", "dest", static_cast<float>((i + 5) % 10) / 10.0f);
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            } catch (...) {
                encounteredError = true;
            }
        });
        
        // Thread for route management
        std::thread routeThread([&]() {
            try {
                for (int i = 0; i < 20 && keepRunning; ++i) {
                    // Add and remove temporary routes
                    std::string tempSource = "temp_source_" + std::to_string(i);
                    std::string tempDest = "temp_dest_" + std::to_string(i);
                    
                    matrix.registerSource(tempSource, "Temp Source", 
                                        [i]() { return static_cast<float>(i % 10) / 10.0f; });
                    
                    matrix.registerDestination(tempDest, "Temp Destination", 
                                             [](float) { /* do nothing */ });
                    
                    matrix.createRoute(tempSource, tempDest, 0.5f);
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    
                    matrix.removeRoute(tempSource, tempDest);
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                }
            } catch (...) {
                encounteredError = true;
            }
        });
        
        // Let threads run for a short time
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        keepRunning = false;
        
        // Join threads
        processingThread.join();
        paramThread.join();
        routeThread.join();
        
        // Verify no errors occurred
        REQUIRE_FALSE(encounteredError);
    }
}

TEST_CASE("Modulation Matrix Preset Tests", "[modulation]") {
    ModulationMatrix matrix(44100.0);
    
    // Create test components
    auto source1 = std::make_shared<MockModulationSource>(0.0f);
    auto source2 = std::make_shared<MockModulationSource>(0.0f);
    auto dest1 = std::make_shared<MockModulationDestination>();
    auto dest2 = std::make_shared<MockModulationDestination>();
    
    // Register components
    matrix.registerSource("source1", "Source 1", [source1]() { return source1->getValue(); });
    matrix.registerSource("source2", "Source 2", [source2]() { return source2->getValue(); });
    matrix.registerDestination("dest1", "Destination 1", 
                             [dest1](float value) { dest1->setValue(value); });
    matrix.registerDestination("dest2", "Destination 2", 
                             [dest2](float value) { dest2->setValue(value); });
    
    SECTION("Create and Load Preset") {
        // Create some routes
        matrix.createRoute("source1", "dest1", 0.7f, ModulationMode::Bipolar, 0.0f);
        matrix.createRoute("source2", "dest2", 0.3f, ModulationMode::Unipolar, 0.2f);
        
        // Create preset
        REQUIRE(matrix.createPreset("TestPreset"));
        
        // Modify routes
        matrix.setRouteDepth("source1", "dest1", 0.1f);
        matrix.removeRoute("source2", "dest2");
        
        // Verify changes took effect
        auto routes = matrix.getAllRoutes();
        REQUIRE(routes.size() == 1);
        REQUIRE(routes[0].depth == Catch::Approx(0.1f));
        
        // Load preset
        REQUIRE(matrix.loadPreset("TestPreset"));
        
        // Verify original configuration restored
        routes = matrix.getAllRoutes();
        REQUIRE(routes.size() == 2);
        
        bool hasRoute1 = false;
        bool hasRoute2 = false;
        
        for (const auto& route : routes) {
            if (route.sourceId == "source1" && route.destinationId == "dest1") {
                hasRoute1 = true;
                REQUIRE(route.depth == Catch::Approx(0.7f));
            }
            if (route.sourceId == "source2" && route.destinationId == "dest2") {
                hasRoute2 = true;
                REQUIRE(route.depth == Catch::Approx(0.3f));
                REQUIRE(route.mode == ModulationMode::Unipolar);
                REQUIRE(route.offset == Catch::Approx(0.2f));
            }
        }
        
        REQUIRE(hasRoute1);
        REQUIRE(hasRoute2);
    }
}