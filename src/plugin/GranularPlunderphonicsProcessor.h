/**
 * @file GranularPlunderphonicsProcessor.h
 * @brief Defines the audio processor component of the GranularPlunderphonics VST3 plugin
 */

#pragma once

#include "../common/Logger.h"
#include "../common/ErrorHandling.h"
#include "GranularPlunderphonicsIDs.h"

// Global forward declarations for VST3 types (outside our namespace)
namespace Steinberg {
    typedef int tresult;
    typedef bool TBool;
    typedef int int32;
    class FUnknown;
    class IBStream;

    namespace Vst {
        struct ProcessData;
        struct SpeakerArrangement;
        typedef float Sample32;
        class IAudioProcessor;
        class IParameterChanges;
        class AudioEffect;
    }
}

// SMTG_OVERRIDE macro replacement
#ifndef SMTG_OVERRIDE
#define SMTG_OVERRIDE override
#endif

// PLUGIN_API macro replacement
#ifndef PLUGIN_API
#define PLUGIN_API virtual
#endif

namespace GranularPlunderphonics {

//------------------------------------------------------------------------
/**
 * @class GranularPlunderphonicsProcessor
 * @brief Main audio processor for the Granular Plunderphonics VST3 plugin
 *
 * This is a placeholder declaration until the VST3 SDK is available.
 * The actual implementation will inherit from Steinberg::Vst::AudioEffect.
 */
class GranularPlunderphonicsProcessor
{
public:
    //------------------------------------------------------------------------
    // Constructor and destructor
    //------------------------------------------------------------------------
    GranularPlunderphonicsProcessor();
    virtual ~GranularPlunderphonicsProcessor();

    //------------------------------------------------------------------------
    // AudioEffect interface stubs (will be implemented when SDK is available)
    //------------------------------------------------------------------------
    /** Called at first after constructor */
    PLUGIN_API ::Steinberg::tresult initialize(::Steinberg::FUnknown* context);

    /** Called after initialize */
    PLUGIN_API ::Steinberg::tresult setActive(::Steinberg::TBool state);

    /** Called before destructor */
    PLUGIN_API ::Steinberg::tresult terminate();

    /** Audio processing */
    PLUGIN_API ::Steinberg::tresult process(::Steinberg::Vst::ProcessData& data);

    /** For persistence */
    PLUGIN_API ::Steinberg::tresult setState(::Steinberg::IBStream* state);
    PLUGIN_API ::Steinberg::tresult getState(::Steinberg::IBStream* state);

    /** Bus arrangement setup */
    PLUGIN_API ::Steinberg::tresult setBusArrangements(
        ::Steinberg::Vst::SpeakerArrangement* inputs, ::Steinberg::int32 numIns,
        ::Steinberg::Vst::SpeakerArrangement* outputs, ::Steinberg::int32 numOuts);

    /** Creation method called by the factory */
    static ::Steinberg::FUnknown* createInstance(void* context);

    //------------------------------------------------------------------------
    // Custom methods
    //------------------------------------------------------------------------
    /** Process mono input to stereo output */
    ::Steinberg::tresult processMonoToStereo(
        ::Steinberg::Vst::ProcessData& data,
        const ::Steinberg::Vst::Sample32* inBuffer,
        ::Steinberg::Vst::Sample32* outLeftBuffer,
        ::Steinberg::Vst::Sample32* outRightBuffer);

    /** Handle parameter changes */
    void processParameterChanges(::Steinberg::Vst::IParameterChanges* paramChanges);

protected:
    //------------------------------------------------------------------------
    // Member variables
    //------------------------------------------------------------------------
    bool mBypass;                // Bypass state
    float mSampleRate;           // Current sample rate
    int mBlockSize;              // Maximum block size
    Logger mLogger;              // Logger instance
    ErrorHandler mErrorHandler;  // Error handling
};

} // namespace GranularPlunderphonics