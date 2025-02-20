#include "GranularPlunderphonicsProcessor.h"
#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

namespace GranularPlunderphonics {

GranularPlunderphonicsProcessor::GranularPlunderphonicsProcessor()
: AudioEffect()
, mBypass(false)
, mSampleRate(44100.0f)
, mBlockSize(1024)
, mLogger("GranularPlunderphonicsProcessor")
{
    mLogger.info("Creating GranularPlunderphonicsProcessor instance");
    setControllerClass(GranularPlunderphonicsControllerUID);
}

Steinberg::tresult PLUGIN_API GranularPlunderphonicsProcessor::initialize(FUnknown* context)
{
    mLogger.info("Initializing processor");

    // First initialize parent class
    Steinberg::tresult result = AudioEffect::initialize(context);
    if (result != Steinberg::kResultOk) {
        mLogger.error("Parent initialization failed");
        return result;
    }

    // Set bus arrangements (1 mono input, 1 stereo output)
    addAudioInput(STR16("Audio Input"), Steinberg::Vst::SpeakerArr::kMono);
    addAudioOutput(STR16("Audio Output"), Steinberg::Vst::SpeakerArr::kStereo);

    return Steinberg::kResultOk;
}

Steinberg::tresult PLUGIN_API GranularPlunderphonicsProcessor::terminate()
{
    mLogger.info("Terminating processor");
    return AudioEffect::terminate();
}

Steinberg::tresult PLUGIN_API GranularPlunderphonicsProcessor::setActive(Steinberg::TBool state)
{
    mLogger.info("Setting processor active state: {}", state ? "active" : "inactive");
    return AudioEffect::setActive(state);
}

Steinberg::tresult PLUGIN_API GranularPlunderphonicsProcessor::process(Steinberg::Vst::ProcessData& data)
{
    // Handle parameter changes
    if (data.inputParameterChanges) {
        processParameterChanges(data.inputParameterChanges);
    }

    // Check if processing is needed
    if (data.numInputs == 0 || data.numOutputs == 0) {
        return Steinberg::kResultOk;
    }

    // Process audio
    return processMonoToStereo(data);
}

Steinberg::tresult PLUGIN_API GranularPlunderphonicsProcessor::setupProcessing(Steinberg::Vst::ProcessSetup& setup)
{
    mLogger.info("Setting up processing: SR={}, MaxBS={}", setup.sampleRate, setup.maxSamplesPerBlock);
    mSampleRate = setup.sampleRate;
    mBlockSize = setup.maxSamplesPerBlock;
    return AudioEffect::setupProcessing(setup);
}

Steinberg::tresult PLUGIN_API GranularPlunderphonicsProcessor::setBusArrangements(
    Steinberg::Vst::SpeakerArrangement* inputs, Steinberg::int32 numIns,
    Steinberg::Vst::SpeakerArrangement* outputs, Steinberg::int32 numOuts)
{
    // Validate bus configuration
    if (numIns == 1 && numOuts == 1 &&
        inputs[0] == Steinberg::Vst::SpeakerArr::kMono &&
        outputs[0] == Steinberg::Vst::SpeakerArr::kStereo) {
        return AudioEffect::setBusArrangements(inputs, numIns, outputs, numOuts);
    }
    return Steinberg::kResultFalse;
}

Steinberg::tresult GranularPlunderphonicsProcessor::processMonoToStereo(Steinberg::Vst::ProcessData& data)
{
    // Get input and output buffers
    Steinberg::Vst::AudioBusBuffers& inputBus = data.inputs[0];
    Steinberg::Vst::AudioBusBuffers& outputBus = data.outputs[0];

    // Get the actual buffers
    float* inBuffer = inputBus.channelBuffers32[0];
    float* outLeftBuffer = outputBus.channelBuffers32[0];
    float* outRightBuffer = outputBus.channelBuffers32[1];

    // Simple mono to stereo pass-through
    for (int32 i = 0; i < data.numSamples; i++) {
        if (mBypass) {
            outLeftBuffer[i] = outRightBuffer[i] = inBuffer[i];
        } else {
            // For now, just copy input to both channels
            // Later we'll add granular processing here
            outLeftBuffer[i] = inBuffer[i];
            outRightBuffer[i] = inBuffer[i];
        }
    }

    return Steinberg::kResultOk;
}

void GranularPlunderphonicsProcessor::processParameterChanges(Steinberg::Vst::IParameterChanges* paramChanges)
{
    // Process bypass parameter changes
    int32 numParamsChanged = paramChanges->getParameterCount();
    for (int32 i = 0; i < numParamsChanged; i++) {
        Steinberg::Vst::IParamValueQueue* paramQueue = paramChanges->getParameterData(i);
        if (paramQueue) {
            Steinberg::Vst::ParamValue value;
            int32 sampleOffset;
            int32 numPoints = paramQueue->getPointCount();
            if (numPoints > 0) {
                paramQueue->getPoint(numPoints - 1, sampleOffset, value);
                switch (paramQueue->getParameterId()) {
                    case kBypassId:
                        mBypass = (value >= 0.5f);
                        break;
                    // Add more parameter cases here as needed
                }
            }
        }
    }
}

} // namespace GranularPlunderphonics
