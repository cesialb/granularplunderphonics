/**
 * @file GranularPlunderphonicsController.h
 * @brief Defines the controller component of the GranularPlunderphonics VST3 plugin
 */

#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "common/Logger.h"
#include "GranularPlunderphonicsIDs.h"

namespace GranularPlunderphonics {

//------------------------------------------------------------------------
/**
 * @class GranularPlunderphonicsController
 * @brief Controller implementation for the Granular Plunderphonics VST3 plugin
 *
 * This class handles parameter management and UI interaction.
 * Currently minimal, will be extended with UI controls for granular parameters.
 */
class GranularPlunderphonicsController : public Steinberg::Vst::EditControllerEx1
{
public:
    //------------------------------------------------------------------------
    // Constructor and destructor
    //------------------------------------------------------------------------
    GranularPlunderphonicsController();
    ~GranularPlunderphonicsController() SMTG_OVERRIDE;

    //------------------------------------------------------------------------
    // EditController overrides
    //------------------------------------------------------------------------
    /** Called at first after constructor */
    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) SMTG_OVERRIDE;

    /** Called before destructor */
    Steinberg::tresult PLUGIN_API terminate() SMTG_OVERRIDE;

    /** For persistence */
    Steinberg::tresult PLUGIN_API setComponentState(Steinberg::IBStream* state) SMTG_OVERRIDE;

    /** Create custom view (editor) */
    Steinberg::IPlugView* PLUGIN_API createView(Steinberg::FIDString name) SMTG_OVERRIDE;

    /** Creation method called by the factory */
    static Steinberg::FUnknown* createInstance(void* /*context*/)
    {
        return (Steinberg::Vst::IEditController*)new GranularPlunderphonicsController;
    }

protected:
    //------------------------------------------------------------------------
    // Member variables
    //------------------------------------------------------------------------
    Logger mLogger;  // Logger instance
};

} // namespace GranularPlunderphonics