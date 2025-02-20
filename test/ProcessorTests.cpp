#include <catch2/catch_test_macros.hpp>
#include "../src/plugin/GranularPlunderphonicsProcessor.h"

TEST_CASE("Plugin Initialization", "[processor]") {
    auto processor = std::make_unique<GranularPlunderphonicsProcessor>();
    REQUIRE(processor != nullptr);

    auto result = processor->initialize(nullptr);
    REQUIRE(result == Steinberg::kResultOk);
}

TEST_CASE("Audio Processing", "[processor]") {
    auto processor = std::make_unique<GranularPlunderphonicsProcessor>();
    processor->initialize(nullptr);

    // Setup test buffers
    std::vector<float> inputBuffer(1024, 0.5f);
    std::vector<float> outputBufferL(1024);
    std::vector<float> outputBufferR(1024);

    // Setup ProcessData
    Steinberg::Vst::ProcessData data;
    // Fill ProcessData structure...

    auto result = processor->process(data);
    REQUIRE(result == Steinberg::kResultOk);

    // Verify audio passed through correctly
    for (size_t i = 0; i < 1024; i++) {
        REQUIRE(outputBufferL[i] == inputBuffer[i]);
        REQUIRE(outputBufferR[i] == inputBuffer[i]);
    }
}

// Add more test cases...
