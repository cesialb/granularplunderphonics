/**
 * @file GranularPlunderphonicsController.h
 * @brief Defines the controller component of the GranularPlunderphonics VST3 plugin
 */

#pragma once

#include <string>
#include "../common/Logger.h"

// Include VST3 SDK headers
#include "base/source/fobject.h"
#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace Steinberg {
    class IBStream;  // Add this line
    typedef const char* FIDString;

    namespace Vst {
        class EditControllerEx1;
        class IEditController;
        class IPlugView;
    }
}

// Plugin-specific forward declarations
namespace GranularPlunderphonics {

// Forward declare error codes here
enum GranularParameters {
    kBypassId = 1000
};

/**
 * @class GranularPlunderphonicsController
 * @brief Controller implementation for the Granular Plunderphonics VST3 plugin
 */
class GranularPlunderphonicsController
{
public:
    //------------------------------------------------------------------------
    // Constructor and destructor
    //------------------------------------------------------------------------
    GranularPlunderphonicsController();
    virtual ~GranularPlunderphonicsController();

    //------------------------------------------------------------------------
    // EditController interface stubs (will be overridden when SDK available)
    //------------------------------------------------------------------------
    ::Steinberg::tresult initialize(::Steinberg::FUnknown* context);
    ::Steinberg::tresult terminate();
    ::Steinberg::tresult setComponentState(::Steinberg::IBStream* state);
    ::Steinberg::Vst::IPlugView* createView(::Steinberg::FIDString name);

    /**
     * @brief Factory method to create instance
     */
    static ::Steinberg::FUnknown* createInstance(void* context);

protected:
    //------------------------------------------------------------------------
    // Member variables
    //------------------------------------------------------------------------
    Logger mLogger;  // Logger instance
};

} // namespace GranularPlunderphonics