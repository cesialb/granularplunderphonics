/**
 * @file ProcessorTests.cpp
 * @brief Unit tests for the GranularPlunderphonicsProcessor
 */

#include <catch2/catch.hpp>
#include "plugin/GranularPlunderphonicsProcessor.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/base/ibstream.h"
#include "public.sdk/source/vst/hosting/processdata.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"
#include "public.sdk/source/vst/utility/memorystream.h"
#include <memory>
#include <vector>

using namespace Steinberg;
using namespace Steinberg::Vst;
using namespace GranularPlunderphonics;

//------------------------------------------------------------------------
// Helper classes and functions
//------------------------------------------------------------------------

// Simple fake factory for testing
class TestFactory : public IPluginFactory {
public:
    tresult PLUGIN_API queryInterface(const TUID _iid, void** obj) override { *obj = nullptr; return kResultFalse; }
    uint32 PLUGIN_API addRef() override { return 1; }
    uint32 PLUGIN_API release() override { return 1; }
    tresult PLUGIN_API getFactoryInfo(PFactoryInfo* info) override { return kNotImplemented; }
    int32 PLUGIN_API countClasses() override { return 0; }
    tresult PLUGIN_API getClassInfo(int32 index, PClassInfo* info) override { return kNotImplemented; }
    tresult PLUGIN_API createInstance(FIDString cid, FIDString _iid, void** obj) override { return kNotImplemented; }
};

// Helper to create test processing buffers
struct TestProcessSetup {
    static constexpr int32 kBlockSize = 512;
    static constexpr float kSampleRate = 44100.0f;

    // Audio buffers
    std::vector<Sample32> inputBuffer;
    std::vector<Sample32> outputLeftBuffer;
    std::vector<Sample32> outputRightBuffer;

    // Pointers for VST3 API
    Sample32* inputPtrs[1];
    Sample32* outputPtrs[2];

    // Bus buffers
    AudioBusBuffers inputBus;
    AudioBusBuffers outputBus;

    // Process data
    ProcessData processData;

    TestProcessSetup()
    : inputBuffer(kBlockSize, 0.0f)
    , outputLeftBuffer