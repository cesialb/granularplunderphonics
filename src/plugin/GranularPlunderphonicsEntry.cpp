/**
* @file GranularPlunderphonicsEntry.cpp
 * @brief Entry point for the GranularPlunderphonics VST3 plugin
 */

#include "GranularPlunderphonicsProcessor.h"
#include "GranularPlunderphonicsController.h"
#include "GranularPlunderphonicsIDs.h"
#include "version.h"

#include "public.sdk/source/main/pluginfactory.h"

#define stringPluginName "Granular Plunderphonics"

using namespace Steinberg;
using namespace Steinberg::Vst;

//------------------------------------------------------------------------
//  VST3 Plugin factory
//------------------------------------------------------------------------
BEGIN_FACTORY_DEF(kGranularPlunderphonicsVendor,
                 kGranularPlunderphonicsURL,
                 kGranularPlunderphonicsEmail)

    //------------------------------------------------------------------------
    // Register the GranularPlunderphonicsProcessor and GranularPlunderphonicsController
    //------------------------------------------------------------------------
    DEF_CLASS2(INLINE_UID_FROM_FUID(GranularPlunderphonics::kGranularPlunderphonicsProcessorUID),
              PClassInfo::kManyInstances,
              kVstAudioEffectClass,
              stringPluginName,
              Vst::kDistributable,
              GranularPlunderphonics::kGranularPlunderphonicsCategory,
              GRANULAR_PLUNDERPHONICS_VERSION_STR,
              kVstVersionString,
              GranularPlunderphonics::GranularPlunderphonicsProcessor::createInstance)

    DEF_CLASS2(INLINE_UID_FROM_FUID(GranularPlunderphonics::kGranularPlunderphonicsControllerUID),
              PClassInfo::kManyInstances,
              kVstComponentControllerClass,
              stringPluginName "Controller",
              0,
              "",
              GRANULAR_PLUNDERPHONICS_VERSION_STR,
              kVstVersionString,
              GranularPlunderphonics::GranularPlunderphonicsController::createInstance)

END_FACTORY