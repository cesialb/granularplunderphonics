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

TEST_CASE("Channel Configuration", "[processor]") {
    auto processor = std::make_unique<GranularPlunderphonicsProcessor>();
    processor->initialize(nullptr);

    // Test mono input to stereo output configuration
    Steinberg::Vst::SpeakerArrangement inputs[1] = { Steinberg::Vst::SpeakerArr::kMono };
    Steinberg::Vst::SpeakerArrangement outputs[1] = { Steinberg::Vst::SpeakerArr::kStereo };

    auto result = processor->setBusArrangements(inputs, 1, outputs, 1);
    REQUIRE(result == Steinberg::kResultOk);
}

TEST_CASE("Resource Cleanup", "[processor]") {
    auto processor = std::make_unique<GranularPlunderphonicsProcessor>();
    processor->initialize(nullptr);

    // Test proper cleanup
    auto result = processor->terminate();
    REQUIRE(result == Steinberg::kResultOk);
}

TEST_CASE("Audio Pass-through", "[processor]") {
    auto processor = std::make_unique<GranularPlunderphonicsProcessor>();
    processor->initialize(nullptr);

    // Create test buffers
    const int blockSize = 512;
    std::vector<float> inputBuffer(blockSize, 0.5f);
    std::vector<float> outputBufferL(blockSize);
    std::vector<float> outputBufferR(blockSize);

    // Setup ProcessData
    Steinberg::Vst::ProcessData data;
    Steinberg::Vst::AudioBusBuffers inputBus;
    Steinberg::Vst::AudioBusBuffers outputBus;

    float* inputChannels[1] = { inputBuffer.data() };
    float* outputChannels[2] = { outputBufferL.data(), outputBufferR.data() };

    inputBus.numChannels = 1;
    inputBus.channelBuffers32 = inputChannels;

    outputBus.numChannels = 2;
    outputBus.channelBuffers32 = outputChannels;

    data.numSamples = blockSize;
    data.inputs = &inputBus;
    data.outputs = &outputBus;
    data.numInputs = 1;
    data.numOutputs = 1;

    // Process audio
    auto result = processor->process(data);
    REQUIRE(result == Steinberg::kResultOk);

    // Verify audio passed through correctly
    for (int i = 0; i < blockSize; i++) {
        REQUIRE(outputBufferL[i] == inputBuffer[i]);
        REQUIRE(outputBufferR[i] == inputBuffer[i]);
    }
}

