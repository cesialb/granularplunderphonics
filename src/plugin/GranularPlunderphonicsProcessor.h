// GranularPlunderphonicsProcessor.h
#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"
#include "pluginterfaces/base/ftypes.h"  // Add this for int32
#include "public.sdk/source/vst/vstaudioeffect.h"
#include "../common/Logger.h"
#include "GranularPlunderphonicsIDs.h"
#include "ParameterManager.h"


namespace GranularPlunderphonics {

    using namespace Steinberg;  // Add this
    using namespace Steinberg::Vst;  // Add this

    class GranularPlunderphonicsProcessor : public AudioEffect {  // Remove Steinberg::Vst:: prefix
    public:
        GranularPlunderphonicsProcessor();
        ~GranularPlunderphonicsProcessor() SMTG_OVERRIDE = default;

        // Create function
        static FUnknown* createInstance(void*) {  // Remove Steinberg:: prefix
            return static_cast<IAudioProcessor*>(new GranularPlunderphonicsProcessor());  // Remove Steinberg::Vst:: prefix
        }

        // AudioEffect overrides
        tresult PLUGIN_API initialize(FUnknown* context) SMTG_OVERRIDE;  // Remove Steinberg:: prefix
        tresult PLUGIN_API terminate() SMTG_OVERRIDE;
        tresult PLUGIN_API setActive(TBool state) SMTG_OVERRIDE;
        tresult PLUGIN_API process(ProcessData& data) SMTG_OVERRIDE;  // Remove Steinberg::Vst:: prefix
        tresult PLUGIN_API setupProcessing(ProcessSetup& setup) SMTG_OVERRIDE;
        tresult PLUGIN_API setBusArrangements(
            SpeakerArrangement* inputs, int32 numIns,  // Remove Steinberg::Vst:: prefix
            SpeakerArrangement* outputs, int32 numOuts) SMTG_OVERRIDE;

    protected:
        void processParameterChanges(IParameterChanges* paramChanges);  // Remove Steinberg::Vst:: prefix

    private:
        bool mBypass;
        float mSampleRate;
        int32 mBlockSize;  // Change int to int32
        Logger mLogger{"GranularProcessor"};
        ParameterManager mParameterManager;
    };

} // namespace GranularPlunderphonics
