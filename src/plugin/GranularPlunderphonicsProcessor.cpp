/**
 * @file GranularPlunderphonicsProcessor.cpp
 * @brief Implementation of the GranularPlunderphonicsProcessor class
 */

#include "GranularPlunderphonicsProcessor.h"
#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/base/ibstream.h"
#include <algorithm>

using namespace Steinberg;
using namespace Steinberg::Vst;

namespace GranularPlunderphonics {

//------------------------------------------------------------------------
// Constructor and destructor
//------------------------------------------------------------------------
GranularPlunderphonicsProcessor::GranularPlunderphonicsProcessor()
: mBypass(false)
, mSampleRate(44100.0f)
, mBlockSize(1024)
, mLogger("GranularPlunderphonicsProcessor")
, mErrorHandler()
{
    mLogger.info("Creating GranularPlunderphonicsProcessor instance");

    // Register parameters
    parameters.addParameter(STR16("Bypass"), nullptr, 1, 0,
                          ParameterInfo::kCanAutomate | ParameterInfo::kIsBypass,
                          GranularParameters::kBypassId);

    // Set the audio channel configurations (mono input, stereo output)
    addAudioInput(STR16("AudioInput"), SpeakerArr::kMono);
    addAudioOutput(STR16("AudioOutput"), SpeakerArr::kStereo);
}

//------------------------------------------------------------------------
GranularPlunderphonicsProcessor::~GranularPlunderphonicsProcessor()
{
    mLogger.info("Destroying GranularPlunderphonicsProcessor instance");
    // Resources are automatically cleaned up thanks to RAII
}

//------------------------------------------------------------------------
// AudioEffect overrides
//------------------------------------------------------------------------
tresult PLUGIN_API GranularPlunderphonicsProcessor::initialize(FUnknown* context)
{
    // First initialize the parent
    tresult result = AudioEffect::initialize(context);
    if (result != kResultOk) {
        mLogger.error("Failed to initialize AudioEffect base class");
        mErrorHandler.setError(ErrorCodes::kInitializationError);
        return result;
    }

    mLogger.info("Initializing GranularPlunderphonicsProcessor");

    // Set initial processing setup
    processSetup.maxSamplesPerBlock = 1024;
    processSetup.processMode = kRealtime;
    processSetup.sampleRate = 44100.0;
    processSetup.symbolicSampleSize = kSample32;

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API GranularPlunderphonicsProcessor::setActive(TBool state)
{
    mLogger.info("Setting processor active state: {}", state ? "active" : "inactive");

    if (state) {
        // Becoming active
        mSampleRate = processSetup.sampleRate;
        mBlockSize = processSetup.maxSamplesPerBlock;

        // Initialize any processing resources here
        mLogger.debug("Sample rate: {}, Block size: {}", mSampleRate, mBlockSize);
    } else {
        // Becoming inactive
        // Clean up any processing resources here
    }

    return AudioEffect::setActive(state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API GranularPlunderphonicsProcessor::terminate()
{
    mLogger.info("Terminating GranularPlunderphonicsProcessor");
    return AudioEffect::terminate();
}

//------------------------------------------------------------------------
tresult PLUGIN_API GranularPlunderphonicsProcessor::process(ProcessData& data)
{
    // Process parameter changes first
    if (data.inputParameterChanges) {
        processParameterChanges(data.inputParameterChanges);
    }

    // Check if processing should be bypassed
    if (mBypass) {
        // Copy input to output if we have audio data
        if (data.numInputs == 0 || data.numOutputs == 0 ||
            data.inputs[0].numChannels == 0 || data.outputs[0].numChannels == 0) {
            return kResultOk;
        }

        // Simply copy input to all output channels
        for (int32 i = 0; i < data.outputs[0].numChannels; i++) {
            if (i < data.inputs[0].numChannels) {
                // Copy input channel to output channel
                memcpy(data.outputs[0].channelBuffers32[i],
                       data.inputs[0].channelBuffers32[i],
                       data.numSamples * sizeof(Sample32));
            } else {
                // Clear additional output channels
                memset(data.outputs[0].channelBuffers32[i], 0,
                       data.numSamples * sizeof(Sample32));
            }
        }
        return kResultOk;
    }

    // Check for valid audio data
    if (data.numInputs == 0 || data.numOutputs == 0) {
        return kResultOk;
    }

    // For mono input to stereo output configuration
    if (data.inputs[0].numChannels == 1 && data.outputs[0].numChannels == 2) {
        const Sample32* inBuffer = data.inputs[0].channelBuffers32[0];
        Sample32* outLeftBuffer = data.outputs[0].channelBuffers32[0];
        Sample32* outRightBuffer = data.outputs[0].channelBuffers32[1];

        return processMonoToStereo(data, inBuffer, outLeftBuffer, outRightBuffer);
    }

    // For other configurations, just pass through
    for (int32 i = 0; i < data.outputs[0].numChannels; i++) {
        if (i < data.inputs[0].numChannels) {
            // Copy input channel to output channel
            memcpy(data.outputs[0].channelBuffers32[i],
                   data.inputs[0].channelBuffers32[i],
                   data.numSamples * sizeof(Sample32));
        } else {
            // Clear additional output channels
            memset(data.outputs[0].channelBuffers32[i], 0,
                   data.numSamples * sizeof(Sample32));
        }
    }

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API GranularPlunderphonicsProcessor::setState(IBStream* state)
{
    mLogger.debug("Setting processor state");

    if (!state) {
        mLogger.error("Null state pointer provided");
        return kInvalidArgument;
    }

    IBStreamer streamer(state, kLittleEndian);

    // Read the bypass state
    int32 bypassState;
    if (streamer.readInt32(bypassState) == false) {
        mLogger.error("Failed to read bypass state");
        return kResultFalse;
    }

    mBypass = (bypassState > 0);
    mLogger.debug("Loaded bypass state: {}", mBypass ? "true" : "false");

    // Additional state parameters will be added here as the plugin develops

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API GranularPlunderphonicsProcessor::getState(IBStream* state)
{
    mLogger.debug("Getting processor state");

    if (!state) {
        mLogger.error("Null state pointer provided");
        return kInvalidArgument;
    }

    IBStreamer streamer(state, kLittleEndian);

    // Write the bypass state
    int32 bypassState = mBypass ? 1 : 0;
    if (streamer.writeInt32(bypassState) == false) {
        mLogger.error("Failed to write bypass state");
        return kResultFalse;
    }

    // Additional state parameters will be added here as the plugin develops

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API GranularPlunderphonicsProcessor::setBusArrangements(
    SpeakerArrangement* inputs, int32 numIns,
    SpeakerArrangement* outputs, int32 numOuts)
{
    mLogger.debug("Setting bus arrangements");

    // Check for required configuration: 1 mono input and 1 stereo output
    if (numIns == 1 && numOuts == 1 &&
        inputs[0] == SpeakerArr::kMono &&
        outputs[0] == SpeakerArr::kStereo) {
        return AudioEffect::setBusArrangements(inputs, numIns, outputs, numOuts);
    }

    mLogger.warn("Unsupported bus arrangement requested");
    return kResultFalse;
}

//------------------------------------------------------------------------
// Custom methods
//------------------------------------------------------------------------
tresult GranularPlunderphonicsProcessor::processMonoToStereo(
    ProcessData& data,
    const Sample32* inBuffer,
    Sample32* outLeftBuffer,
    Sample32* outRightBuffer)
{
    // Simple mono to stereo pass-through for now
    // This will be replaced with actual granular processing later

    if (!inBuffer || !outLeftBuffer || !outRightBuffer) {
        mLogger.error("Null buffer pointer in processMonoToStereo");
        mErrorHandler.setError(ErrorCodes::kProcessingError);
        return kInvalidArgument;
    }

    try {
        // Copy input to both left and right channels
        for (int32 i = 0; i < data.numSamples; i++) {
            outLeftBuffer[i] = inBuffer[i];
            outRightBuffer[i] = inBuffer[i];
        }
    } catch (const std::exception& e) {
        mLogger.error("Exception in audio processing: {}", e.what());
        mErrorHandler.setError(ErrorCodes::kProcessingError);
        return kInternalError;
    }

    return kResultOk;
}

//------------------------------------------------------------------------
void GranularPlunderphonicsProcessor::processParameterChanges(IParameterChanges* paramChanges)
{
    if (!paramChanges) {
        return;
    }

    int32 numParams = paramChanges->getParameterCount();

    // For each parameter that changed
    for (int32 i = 0; i < numParams; i++) {
        IParamValueQueue* paramQueue = paramChanges->getParameterData(i);
        if (!paramQueue) {
            continue;
        }

        int32 numPoints = paramQueue->getPointCount();
        if (numPoints <= 0) {
            continue;
        }

        int32 sampleOffset;
        ParamValue value;

        // Get the last change (we're just doing a simple parameter value update)
        if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) != kResultTrue) {
            continue;
        }

        ParamID paramId = paramQueue->getParameterId();

        // Handle the parameter change
        switch (paramId) {
            case GranularParameters::kBypassId:
                mBypass = (value >= 0.5);
                mLogger.debug("Bypass parameter changed to: {}", mBypass ? "on" : "off");
                break;

            // Additional parameters will be handled here

            default:
                break;
        }
    }
}

} // namespace GranularPlunderphonics