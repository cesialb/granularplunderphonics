/**
 * @file GranularPlunderphonicsProcessor.h
 * @brief Defines the audio processor component of the GranularPlunderphonics VST3 plugin
 */

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"
#include "public.sdk/source/vst/vstaudioeffect.h"
#include "../common/Logger.h"
#include "GranularPlunderphonicsIDs.h"

namespace GranularPlunderphonics {

//------------------------------------------------------------------------
/**
 * @class GranularPlunderphonicsProcessor
 * @brief Main audio processor for the Granular Plunderphonics VST3 plugin
 */
class GranularPlunderphonicsProcessor : public Steinberg::Vst::AudioEffect
{
public:
    //------------------------------------------------------------------------
    // Constructor and destructor
    //------------------------------------------------------------------------
    GranularPlunderphonicsProcessor();

    //------------------------------------------------------------------------
    // AudioEffect interface implementation
    //------------------------------------------------------------------------
    /** Called at first after constructor */
    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) SMTG_OVERRIDE;

    /** Called after initialize */
    Steinberg::tresult PLUGIN_API setActive(Steinberg::TBool state) SMTG_OVERRIDE;

    /** Called before destructor */
    Steinberg::tresult PLUGIN_API terminate() SMTG_OVERRIDE;

    /** Audio processing */
    Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData& data) SMTG_OVERRIDE;

    /** Setup processing */
    Steinberg::tresult PLUGIN_API setupProcessing(Steinberg::Vst::ProcessSetup& setup) SMTG_OVERRIDE;

    /** Bus arrangement setup */
    Steinberg::tresult PLUGIN_API setBusArrangements(
        Steinberg::Vst::SpeakerArrangement* inputs, Steinberg::int32 numIns,
        Steinberg::Vst::SpeakerArrangement* outputs, Steinberg::int32 numOuts) SMTG_OVERRIDE;

    /** Creation method called by the factory */
    static Steinberg::FUnknown* createInstance(void* context);

protected:
    //------------------------------------------------------------------------
    // Member variables
    //------------------------------------------------------------------------
    bool mBypass;         // Bypass state
    float mSampleRate;    // Current sample rate
    int mBlockSize;       // Maximum block size
    Logger mLogger;       // Logger instance

    //------------------------------------------------------------------------
    // Helper methods
    //------------------------------------------------------------------------
    /** Handle parameter changes */
    void processParameterChanges(Steinberg::Vst::IParameterChanges* paramChanges);
};

} // namespace GranularPlunderphonics