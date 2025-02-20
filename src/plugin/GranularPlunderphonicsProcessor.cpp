#include "GranularPlunderphonicsProcessor.h"
#include "GranularPlunderphonicsIDs.h"
#include "GranularParameters.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"
#include "pluginterfaces/vst/vstspeaker.h"

namespace GranularPlunderphonics {

//------------------------------------------------------------------------
// Constructor/Destructor
//------------------------------------------------------------------------
GranularPlunderphonicsProcessor::GranularPlunderphonicsProcessor()
    : mLogger("GranularPlunderphonicsProcessor")
    , mBypass(false)
    , mSampleRate(44100.0f)
    , mBlockSize(1024)
{
    mLogger.info("Creating GranularPlunderphonicsProcessor instance");
}

//------------------------------------------------------------------------
Steinberg::tresult PLUGIN_API GranularPlunderphonicsProcessor::initialize(::Steinberg::FUnknown* context)
{
    mLogger.info("Initializing processor");
    return Steinberg::kResultOk;
}

//------------------------------------------------------------------------
Steinberg::tresult PLUGIN_API GranularPlunderphonicsProcessor::terminate()
{
    mLogger.info("Terminating processor");
    return Steinberg::kResultOk;
}

//------------------------------------------------------------------------
Steinberg::tresult PLUGIN_API GranularPlunderphonicsProcessor::setActive(::Steinberg::TBool state)
{
    std::string message = "Plugin ";
    message += state ? "activated" : "deactivated";
    mLogger.info(message.c_str());
    return Steinberg::kResultOk;
}

//------------------------------------------------------------------------
Steinberg::tresult PLUGIN_API GranularPlunderphonicsProcessor::process(::Steinberg::Vst::ProcessData& data)
{
    // Process parameter changes
    if (data.inputParameterChanges) {
        processParameterChanges(data.inputParameterChanges);
    }

    // Skip processing if there's no audio data or if bypassed
    if (!data.numSamples || mBypass) {
        return Steinberg::kResultOk;
    }

    // Get audio buffers
    if (data.numInputs <= 0 || data.numOutputs <= 0) {
        return Steinberg::kResultOk;
    }

    // Get input buffer
    ::Steinberg::Vst::AudioBusBuffers& inputBus = data.inputs[0];
    if (inputBus.numChannels <= 0) {
        return Steinberg::kResultOk;
    }

    // Get output buffer
    ::Steinberg::Vst::AudioBusBuffers& outputBus = data.outputs[0];
    if (outputBus.numChannels <= 0) {
        return Steinberg::kResultOk;
    }

    // Simple mono to stereo passthrough
    if (inputBus.numChannels == 1 && outputBus.numChannels == 2) {
        float* inputChannel = inputBus.channelBuffers32[0];
        float* outLeftChannel = outputBus.channelBuffers32[0];
        float* outRightChannel = outputBus.channelBuffers32[1];

        // Copy input to both output channels
        for (int i = 0; i < data.numSamples; i++) {
            outLeftChannel[i] = inputChannel[i];
            outRightChannel[i] = inputChannel[i];
        }
    }
    // Simple stereo passthrough
    else if (inputBus.numChannels == 2 && outputBus.numChannels == 2) {
        float* inLeftChannel = inputBus.channelBuffers32[0];
        float* inRightChannel = inputBus.channelBuffers32[1];
        float* outLeftChannel = outputBus.channelBuffers32[0];
        float* outRightChannel = outputBus.channelBuffers32[1];

        // Copy input to output
        for (int i = 0; i < data.numSamples; i++) {
            outLeftChannel[i] = inLeftChannel[i];
            outRightChannel[i] = inRightChannel[i];
        }
    }

    return Steinberg::kResultOk;
}

//------------------------------------------------------------------------
Steinberg::tresult PLUGIN_API GranularPlunderphonicsProcessor::setupProcessing(::Steinberg::Vst::ProcessSetup& setup)
{
    std::string setupMessage = "Setting up processing: sampleRate=" +
                              std::to_string(setup.sampleRate) +
                              ", maxSamplesPerBlock=" +
                              std::to_string(setup.maxSamplesPerBlock);
    mLogger.info(setupMessage.c_str());

    // Store processing setup info
    mSampleRate = setup.sampleRate;
    mBlockSize = setup.maxSamplesPerBlock;

    return Steinberg::kResultOk;
}

//------------------------------------------------------------------------
Steinberg::tresult PLUGIN_API GranularPlunderphonicsProcessor::setBusArrangements(
    ::Steinberg::Vst::SpeakerArrangement* inputs, ::Steinberg::int32 numIns,
    ::Steinberg::Vst::SpeakerArrangement* outputs, ::Steinberg::int32 numOuts)
{
    // Accept mono input to stereo output
    // Using hardcoded values to avoid any symbol resolution issues
    // 0x01 = Mono (Center), 0x03 = Stereo (L+R)
    if (numIns == 1 && numOuts == 1 && inputs[0] == 0x01 && outputs[0] == 0x03) {
        return Steinberg::kResultOk;
    }

    // Accept stereo input to stereo output
    if (numIns == 1 && numOuts == 1 && inputs[0] == 0x03 && outputs[0] == 0x03) {
        return Steinberg::kResultOk;
    }

    // Any other configuration is not supported
    return Steinberg::kResultFalse;
}

//------------------------------------------------------------------------
void GranularPlunderphonicsProcessor::processParameterChanges(::Steinberg::Vst::IParameterChanges* paramChanges)
{
    if (!paramChanges) {
        return;
    }

    // Get parameter count
    int numParamsChanged = paramChanges->getParameterCount();

    // For each parameter change
    for (int i = 0; i < numParamsChanged; i++) {
        ::Steinberg::Vst::IParamValueQueue* paramQueue = paramChanges->getParameterData(i);
        if (!paramQueue) {
            continue;
        }

        // Get parameter ID
        int paramId = paramQueue->getParameterId();

        // Get last point value (most recent change)
        int pointCount = paramQueue->getPointCount();
        if (pointCount <= 0) {
            continue;
        }

        double value;
        int sampleOffset;

        if (paramQueue->getPoint(pointCount - 1, sampleOffset, value) == 0) { // 0 is kResultOk
            // Set parameter value using string concatenation
            std::string logMessage = "Parameter change: ID=" + std::to_string(paramId) +
                                   ", value=" + std::to_string(value);
            mLogger.debug(logMessage.c_str());

            // Handle specific parameters
            if (paramId == kBypassId) {
                mBypass = (value >= 0.5f);
            }
            // Add handling for other parameters as needed
        }
    }
}

//------------------------------------------------------------------------
::Steinberg::FUnknown* GranularPlunderphonicsProcessor::createInstance(void* /*context*/)
{
    return (::Steinberg::FUnknown*)(new GranularPlunderphonicsProcessor());
}

} // namespace GranularPlunderphonics