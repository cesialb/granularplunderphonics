// GranularPlunderphonicsProcessor.cpp
#include "GranularPlunderphonicsProcessor.h"
#include "GranularPlunderphonicsIDs.h"
#include "GranularParameters.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"
#include "pluginterfaces/vst/vstspeaker.h"
#include "../audio/LorenzAttractor.h"
#include "../audio/ModulationMatrixFactory.h"

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

        // Initialize attractors
        mAttractors["lorenz"] = std::make_shared<LorenzAttractor>(mSampleRate);

        // Initialize cloud parameters
        mCloudParams.density = 10.0f;
        mCloudParams.spread = 0.5f;
        mCloudParams.overlap = 0.5f;
        mCloudParams.positionRange = 1.0f;
        mCloudParams.positionOffset = 0.0f;
    }

    //------------------------------------------------------------------------
    tresult PLUGIN_API GranularPlunderphonicsProcessor::initialize(::Steinberg::FUnknown* context)
    {
        // First call parent implementation
        tresult result = AudioEffect::initialize(context);
        if (result != kResultOk) {
            return result;
        }

        // Register parameters with parameter manager
        GranularParameters::registerParameters(mParameterManager);

        // Create modulation matrix
        mModulationMatrix = ModulationMatrixFactory::createStandardMatrix(
            mParameterManager, mAttractors, mCloudParams, mSampleRate);

        mLogger.info("Modulation matrix initialized");

        return kResultOk;
    }

//------------------------------------------------------------------------
tresult PLUGIN_API GranularPlunderphonicsProcessor::terminate()
{
    mLogger.info("Terminating processor");
    return AudioEffect::terminate();
}

//------------------------------------------------------------------------
    tresult PLUGIN_API GranularPlunderphonicsProcessor::setActive(::Steinberg::TBool state)
    {
        if (state) {
            mLogger.info("Processor activated");

            // Reset modulation smoothing when activated
            if (mModulationMatrix) {
                mModulationMatrix->resetSmoothing();
            }

            // Reset attractors
            for (auto& [id, attractor] : mAttractors) {
                attractor->reset();
            }
        } else {
            mLogger.info("Processor deactivated");
        }

        return AudioEffect::setActive(state);
    }

//------------------------------------------------------------------------
    tresult PLUGIN_API GranularPlunderphonicsProcessor::process(ProcessData& data)
{
    // Process parameter changes first
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

    // Process attractors
    for (auto& [id, attractor] : mAttractors) {
        // For Lorenz attractor, we need to process once per audio block
        // This provides chaotic modulation signals
        attractor->process();
    }

    // Process control-rate modulation
    if (mModulationMatrix) {
        mModulationMatrix->processControlRateModulation();
    }

    // Get audio buffers
    AudioBusBuffers& inputBus = data.inputs[0];
    AudioBusBuffers& outputBus = data.outputs[0];

    // Check channel configuration
    if (inputBus.numChannels == 0 || outputBus.numChannels == 0) {
        return kResultOk;
    }

    // Process audio - sample by sample to allow audio-rate modulation
    for (::Steinberg::int32 sample = 0; sample < data.numSamples; sample++) {
        // Process audio-rate modulation for this sample
        if (mModulationMatrix) {
            mModulationMatrix->processAudioRateModulation(sample, data.numSamples);
        }

        // Process each channel
        for (::Steinberg::int32 channel = 0; channel < inputBus.numChannels && channel < outputBus.numChannels; channel++) {
            float* in = inputBus.channelBuffers32[channel];
            float* out = outputBus.channelBuffers32[channel];

            if (in && out) {
                // Apply any audio-rate modulation effects here
                out[sample] = in[sample];
            }
        }
    }

    // If we have a stereo output but mono input, copy the first channel to the second
    if (outputBus.numChannels > 1 && inputBus.numChannels == 1) {
        if (outputBus.channelBuffers32[0] && outputBus.channelBuffers32[1]) {
            for (::Steinberg::int32 sample = 0; sample < data.numSamples; ++sample) {
                outputBus.channelBuffers32[1][sample] = outputBus.channelBuffers32[0][sample];
            }
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

        ::Steinberg::int32 numParamsChanged = paramChanges->getParameterCount();

        // For each parameter change
        for (::Steinberg::int32 i = 0; i < numParamsChanged; ++i) {
            ::Steinberg::Vst::IParamValueQueue* paramQueue = paramChanges->getParameterData(i);
            if (!paramQueue) {
                continue;
            }

            ::Steinberg::Vst::ParamID paramId = paramQueue->getParameterId();
            ::Steinberg::int32 numPoints = paramQueue->getPointCount();
            ::Steinberg::int32 sampleOffset;
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

        // Update modulation matrix sample rate
        if (mModulationMatrix) {
            mModulationMatrix->setSampleRate(mSampleRate);
        }

        // Update attractors sample rate
        for (auto& [id, attractor] : mAttractors) {
            if (auto lorenzAttractor = std::dynamic_pointer_cast<LorenzAttractor>(attractor)) {
                // Assuming LorenzAttractor has a sampleRate parameter in constructor
                // If not, you'll need to add a setSampleRate method
                lorenzAttractor->setUpdateRate(1.0 / mSampleRate);
            }
        }

        return AudioEffect::setupProcessing(setup);
    }

    //------------------------------------------------------------------------
    tresult PLUGIN_API GranularPlunderphonicsProcessor::setBusArrangements(
        SpeakerArrangement* inputs, ::Steinberg::int32 numIns,
        SpeakerArrangement* outputs, ::Steinberg::int32 numOuts)
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