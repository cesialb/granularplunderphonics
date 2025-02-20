#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "public.sdk/source/vst/vstaudioeffect.h"
#include "../common/Logger.h"
#include "../common/ErrorHandling.h"
#include "GranularPlunderphonicsIDs.h"

using namespace Steinberg;  // Add this to use Steinberg namespace directly

namespace GranularPlunderphonics {

    class GranularPlunderphonicsProcessor : public Vst::AudioEffect {
    public:
        GranularPlunderphonicsProcessor();
        virtual ~GranularPlunderphonicsProcessor() = default;

        static FUnknown* createInstance(void*) {
            return (Vst::IAudioProcessor*)new GranularPlunderphonicsProcessor();
        }

        //------------------------------------------------------------------------
        // AudioEffect interface implementation
        //------------------------------------------------------------------------
        tresult PLUGIN_API initialize(FUnknown* context) SMTG_OVERRIDE;
        tresult PLUGIN_API terminate() SMTG_OVERRIDE;
        tresult PLUGIN_API setActive(TBool state) SMTG_OVERRIDE;
        tresult PLUGIN_API process(Vst::ProcessData& data) SMTG_OVERRIDE;
        tresult PLUGIN_API setupProcessing(Vst::ProcessSetup& setup) SMTG_OVERRIDE;
        tresult PLUGIN_API setBusArrangements(Vst::SpeakerArrangement* inputs,
                                            int32 numIns,
                                            Vst::SpeakerArrangement* outputs,
                                            int32 numOuts) SMTG_OVERRIDE;

    protected:
        //------------------------------------------------------------------------
        // Custom methods
        //------------------------------------------------------------------------
        tresult processMonoToStereo(Vst::ProcessData& data);
        void processParameterChanges(Vst::IParameterChanges* paramChanges);

    private:
        bool mBypass;
        float mSampleRate;
        int mBlockSize;
        Logger mLogger;
        ErrorHandler mErrorHandler;
    };

} // namespace GranularPlunderphonics
