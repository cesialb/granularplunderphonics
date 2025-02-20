#include "GranularPlunderphonicsProcessor.h"

namespace GranularPlunderphonics {

    GranularPlunderphonicsProcessor::GranularPlunderphonicsProcessor()
        : AudioEffect()
        , mLogger("GranularPlunderphonicsProcessor")
    {
        mLogger.info("Creating GranularPlunderphonicsProcessor instance");
        setControllerClass(kGranularPlunderphonicsControllerUID);
    }

    Steinberg::tresult PLUGIN_API GranularPlunderphonicsProcessor::initialize(Steinberg::FUnknown* context)
    {
        mLogger.info("Initializing processor");

        Steinberg::tresult result = AudioEffect::initialize(context);
        if (result != Steinberg::kResultOk) {
            mLogger.error("Parent initialization failed");
            return result;
        }

        addAudioInput(STR16("Audio Input"), Steinberg::Vst::SpeakerArr::kMono);
        addAudioOutput(STR16("Audio Output"), Steinberg::Vst::SpeakerArr::kStereo);

        return Steinberg::kResultOk;
    }

    // ... (rest of the implementation)

} // namespace GranularPlunderphonics
