// GranularPlunderphonicsProcessor.cpp
#include "GranularPlunderphonicsProcessor.h"
#include "GranularPlunderphonicsIDs.h"
#include "GranularParameters.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"
#include "pluginterfaces/vst/vstspeaker.h"

namespace GranularPlunderphonics {

    using namespace Steinberg;  // Add this
    using namespace ::Steinberg::Vst;  // Add this

    //------------------------------------------------------------------------
    GranularPlunderphonicsProcessor::GranularPlunderphonicsProcessor()
        : AudioEffect()
        , mLogger("GranularProcessor")
        , mBypass(false)
        , mSampleRate(44100.0f)
        , mBlockSize(1024)
    {
        mLogger.info("Creating GranularPlunderphonicsProcessor instance");
        setControllerClass(kGranularPlunderphonicsControllerUID);
    }

    //------------------------------------------------------------------------
    tresult PLUGIN_API GranularPlunderphonicsProcessor::initialize(FUnknown* context)
    {
        // Call parent class initialization first
        tresult result = AudioEffect::initialize(context);
        if (result != kResultOk) {
            return result;
        }

        // Add audio inputs and outputs
        addAudioInput(STR16("Stereo In"), SpeakerArr::kStereo);
        addAudioOutput(STR16("Stereo Out"), SpeakerArr::kStereo);

        mLogger.info("Processor initialized");
        return kResultOk;
    }

//------------------------------------------------------------------------
tresult PLUGIN_API GranularPlunderphonicsProcessor::terminate()
{
    mLogger.info("Terminating processor");
    return AudioEffect::terminate();
}

//------------------------------------------------------------------------
tresult PLUGIN_API GranularPlunderphonicsProcessor::setActive(TBool state)
{
    if (state) {
        mLogger.info("Processor activated");
    } else {
        mLogger.info("Processor deactivated");
    }
    return AudioEffect::setActive(state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API GranularPlunderphonicsProcessor::process(ProcessData& data)
{
    // Process parameter changes
    if (data.inputParameterChanges) {
        processParameterChanges(data.inputParameterChanges);
    }

    // Skip processing if there's no audio data or if bypassed
    if (!data.numSamples || mBypass) {
        return kResultOk;
    }

    // Check for valid inputs/outputs
    if (data.numInputs == 0 || data.numOutputs == 0) {
        return kResultOk;
    }

    // Get audio buffers
    AudioBusBuffers& inputBus = data.inputs[0];
    AudioBusBuffers& outputBus = data.outputs[0];

    // Check channel configuration
    if (inputBus.numChannels == 0 || outputBus.numChannels == 0) {
        return kResultOk;
    }

    // Process audio
    // For now, just copy input to output (bypass mode)
    for (int32 channel = 0; channel < inputBus.numChannels && channel < outputBus.numChannels; channel++) {
        float* in = inputBus.channelBuffers32[channel];
        float* out = outputBus.channelBuffers32[channel];

        if (in && out) {
            memcpy(out, in, data.numSamples * sizeof(float));
        }
    }

    return kResultOk;
}

    // Update this implementation in GranularPlunderphonicsProcessor.cpp
    void GranularPlunderphonicsProcessor::processParameterChanges(::Steinberg::Vst::IParameterChanges* paramChanges)
    {
        if (!paramChanges) {
            return;
        }

        int32 numParamsChanged = paramChanges->getParameterCount();

        // For each parameter change
        for (int32 i = 0; i < numParamsChanged; i++) {
            ::Steinberg::Vst::IParamValueQueue* paramQueue = paramChanges->getParameterData(i);
            if (!paramQueue) {
                continue;
            }

            ::Steinberg::Vst::ParamID paramId = paramQueue->getParameterId();
            int32 numPoints = paramQueue->getPointCount();
            int32 sampleOffset;
            ::Steinberg::Vst::ParamValue value;

            // Get the last point in the queue
            if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) != ::Steinberg::kResultTrue) {
                continue;
            }

            // Handle parameter changes
            switch (paramId) {
                case GranularPlunderphonics::kBypassId:
                    mBypass = (value >= 0.5f);
                break;

                case GranularPlunderphonics::kGrainSizeId:
                    // Store grain size parameter value
                        // You'll implement grain size handling here
                            break;

                case GranularPlunderphonics::kGrainShapeId:
                    // Store grain shape parameter value
                        // You'll implement grain shape handling here
                            break;

                case GranularPlunderphonics::kGrainDensityId:
                    // Store grain density parameter value
                        // You'll implement grain density handling here
                            break;

                default:
                    break;
            }
        }
    }

//------------------------------------------------------------------------
tresult PLUGIN_API GranularPlunderphonicsProcessor::setupProcessing(ProcessSetup& setup)
{
    mLogger.info(("Setting up processing: sampleRate=" + std::to_string(setup.sampleRate) +
                 ", maxSamplesPerBlock=" + std::to_string(setup.maxSamplesPerBlock)).c_str());

    mSampleRate = static_cast<float>(setup.sampleRate);
    mBlockSize = setup.maxSamplesPerBlock;

    return AudioEffect::setupProcessing(setup);
}

    //------------------------------------------------------------------------
    tresult PLUGIN_API GranularPlunderphonicsProcessor::setBusArrangements(
        SpeakerArrangement* inputs, int32 numIns,
        SpeakerArrangement* outputs, int32 numOuts)
    {
        // Accept mono input to stereo output
        if (numIns == 1 && numOuts == 1 &&
            inputs[0] == SpeakerArr::kMono &&
            outputs[0] == SpeakerArr::kStereo)
        {
            return AudioEffect::setBusArrangements(inputs, numIns, outputs, numOuts);
        }

        // Accept stereo input to stereo output
        if (numIns == 1 && numOuts == 1 &&
            inputs[0] == SpeakerArr::kStereo &&
            outputs[0] == SpeakerArr::kStereo)
        {
            return AudioEffect::setBusArrangements(inputs, numIns, outputs, numOuts);
        }

        return kResultFalse;
    }

} // namespace GranularPlunderphonics
