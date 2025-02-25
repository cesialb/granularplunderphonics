#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/base/ftypes.h"
#include "pluginterfaces/vst/vsttypes.h"
#include "public.sdk/source/vst/vstaudioeffect.h"
#include "../common/Logger.h"
#include "GranularPlunderphonicsIDs.h"
#include "ParameterManager.h"
#include "../audio/ModulationMatrix.h"
#include "../audio/ChaoticBase.h"
#include "../audio/GrainCloud.h"

namespace GranularPlunderphonics {

    using namespace Steinberg;
    using namespace ::Steinberg::Vst;

    class GranularPlunderphonicsProcessor : public Vst::AudioEffect {
    public:
        GranularPlunderphonicsProcessor();
        ~GranularPlunderphonicsProcessor() SMTG_OVERRIDE = default;

        static ::Steinberg::FUnknown* createInstance(void*) {
            return static_cast<Vst::IAudioProcessor*>(new GranularPlunderphonicsProcessor());
        }

        // AudioEffect overrides
        tresult PLUGIN_API initialize(::Steinberg::FUnknown* context) SMTG_OVERRIDE;
        tresult PLUGIN_API terminate() SMTG_OVERRIDE;
        tresult PLUGIN_API setActive(::Steinberg::TBool state) SMTG_OVERRIDE;
        tresult PLUGIN_API process(Vst::ProcessData& data) SMTG_OVERRIDE;
        tresult PLUGIN_API setupProcessing(Vst::ProcessSetup& setup) SMTG_OVERRIDE;
        tresult PLUGIN_API setBusArrangements(
            Vst::SpeakerArrangement* inputs, ::Steinberg::int32 numIns,
            Vst::SpeakerArrangement* outputs, ::Steinberg::int32 numOuts) SMTG_OVERRIDE;

    protected:
        void processParameterChanges(Vst::IParameterChanges* paramChanges);

    private:
        bool mBypass;
        float mSampleRate;
        ::Steinberg::int32 mBlockSize;
        Logger mLogger{"GranularProcessor"};
        ParameterManager mParameterManager;
        std::shared_ptr<ModulationMatrix> mModulationMatrix;
        std::map<std::string, std::shared_ptr<ChaoticAttractor>> mAttractors;
        CloudParameters mCloudParams;
    };

} // namespace GranularPlunderphonics
