#include "GranularPlunderphonicsProcessor.h"
#include "GranularPlunderphonicsController.h"
#include "GranularPlunderphonicsIDs.h"
#include "version.h"

#include "public.sdk/source/main/pluginfactory.h"

// Plugin information
#define stringPluginName "Granular Plunderphonics"
#define stringVendor "YourCompanyName"
#define stringURL "www.yourcompany.com"
#define stringEmail "support@yourcompany.com"

#ifndef kVstComponentControllerClass
#define kVstComponentControllerClass "Component Controller Class"
#endif

using namespace Steinberg;

//------------------------------------------------------------------------
//  VST Plugin Entry
//------------------------------------------------------------------------
// Windows: do not forget to include a .def file in your project to export
// GetPluginFactory function!
//------------------------------------------------------------------------

BEGIN_FACTORY_DEF(stringVendor,
                  stringURL,
                  stringEmail)

    // Register the Processor
    DEF_CLASS2(INLINE_UID_FROM_FUID(GranularPlunderphonics::kGranularPlunderphonicsProcessorUID),
               PClassInfo::kManyInstances,  // Multiple instances supported
               kVstAudioEffectClass,        // Category
               stringPluginName,            // Name
               Vst::kDistributable,         // Component Flags
               Vst::PlugType::kFxGenerator, // Subcategory
               FULL_VERSION_STR,            // Version
               kVstVersionString,           // SDK Version
               GranularPlunderphonics::GranularPlunderphonicsProcessor::createInstance)

    // Register the Controller
    DEF_CLASS2(INLINE_UID_FROM_FUID(GranularPlunderphonics::kGranularPlunderphonicsControllerUID),
               PClassInfo::kManyInstances,  // Multiple instances supported
               kVstComponentControllerClass, // Category
               stringPluginName "Controller", // Name
               0,                           // Component Flags
               "",                          // Subcategory
               FULL_VERSION_STR,            // Version
               kVstVersionString,           // SDK Version
               GranularPlunderphonics::GranularPlunderphonicsController::createInstance)

END_FACTORY