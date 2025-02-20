#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/vsttypes.h"
#include "../common/Logger.h"
#include "../common/ErrorHandling.h"
#include "GranularPlunderphonicsIDs.h"

namespace GranularPlunderphonics {

class GranularPlunderphonicsProcessor : public Steinberg::Vst::AudioEffect {
public:
    // Constructor/Destructor
    GranularPlunderphonicsProcessor();
    ~GranularPlunderphonicsProcessor() SMTG_OVERRIDE = default;

    // Factory method for VST3
    static Steinberg::FUnknown* createInstance(void*) {
        return (Steinberg::Vst::IAudioProcessor*)new GranularPlunderphonicsProcessor();
    }

    // AudioEffect overrides
    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API terminate() SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API setActive(Steinberg::TBool state) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData& data) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API setupProcessing(Steinberg::Vst::ProcessSetup& setup) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API setBusArrangements(Steinberg::Vst::SpeakerArrangement* inputs,
                                                    Steinberg::int32 numIns,
                                                    Steinberg::Vst::SpeakerArrangement* outputs,
                                                    Steinberg::int32 numOuts) SMTG_OVERRIDE;

protected:
    // Helper methods
    Steinberg::tresult processMonoToStereo(Steinberg::Vst::ProcessData& data);
    void processParameterChanges(Steinberg::Vst::IParameterChanges* paramChanges);

private:
    bool mBypass;
    float mSampleRate;
    int mBlockSize;
    Logger mLogger;
    ErrorHandler mErrorHandler;

    DEFINE_INTERFACES
    DEF_INTERFACE(Steinberg::Vst::IAudioProcessor)
    END_DEFINE_INTERFACES
};

} // namespace GranularPlunderphonics
