/**
 * @file GranularPlunderphonicsProcessor.h
 * @brief Defines the audio processor component of the GranularPlunderphonics VST3 plugin
 */

#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "common/Logger.h"
#include "common/ErrorHandling.h"
#include "GranularPlunderphonicsIDs.h"

namespace GranularPlunderphonics {

//------------------------------------------------------------------------
/**
 * @class GranularPlunderphonicsProcessor
 * @brief Main audio processor for the Granular Plunderphonics VST3 plugin
 * 
 * This class handles the audio processing, implementing a mono-to-stereo
 * configuration with basic passthrough functionality. It will be extended
 * with granular processing capabilities in future versions.
 */
class GranularPlunderphonicsProcessor : public Steinberg::Vst::AudioEffect
{
public:
    //------------------------------------------------------------------------
    // Constructor and destructor
    //------------------------------------------------------------------------
    GranularPlunderphonicsProcessor();
    ~GranularPlunderphonicsProcessor() SMTG_OVERRIDE;

    //------------------------------------------------------------------------
    // AudioEffect overrides
    //------------------------------------------------------------------------
    /** Called at first after constructor */
    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) SMTG_OVERRIDE;
    
    /** Called after initialize */
    Steinberg::tresult PLUGIN_API setActive(Steinberg::TBool state) SMTG_OVERRIDE;
    
    /** Called before destructor */
    Steinberg::tresult PLUGIN_API terminate() SMTG_OVERRIDE;
    
    /** Audio processing */
    Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData& data) SMTG_OVERRIDE;
    
    /** For persistence */
    Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) SMTG_OVERRIDE;
    
    /** Bus arrangement setup */
    Steinberg::tresult PLUGIN_API setBusArrangements(
        Steinberg::Vst::SpeakerArrangement* inputs, Steinberg::int32 numIns,
        Steinberg::Vst::SpeakerArrangement* outputs, Steinberg::int32 numOuts) SMTG_OVERRIDE;
    
    /** Creation method called by the factory */
    static Steinberg::FUnknown* createInstance(void* /*context*/)
    {
        return (Steinberg::Vst::IAudioProcessor*)new GranularPlunderphonicsProcessor;
    }

    //------------------------------------------------------------------------
    // Custom methods
    //------------------------------------------------------------------------
    /** Process mono input to stereo output */
    Steinberg::tresult processMonoToStereo(
        Steinberg::Vst::ProcessData& data,
        const Steinberg::Vst::Sample32* inBuffer,
        Steinberg::Vst::Sample32* outLeftBuffer,
        Steinberg::Vst::Sample32* outRightBuffer);
    
    /** Handle parameter changes */
    void processParameterChanges(Steinberg::Vst::IParameterChanges* paramChanges);

protected:
    //------------------------------------------------------------------------
    // Member variables
    //------------------------------------------------------------------------
    bool mBypass;                // Bypass state
    float mSampleRate;           // Current sample rate
    int mBlockSize;              // Maximum block size
    Logger mLogger;              // Logger instance
    ErrorHandler mErrorHandler;  // Error handling
    
    // Additional state will be added for granular processing
};

} // namespace GranularPlunderphonics