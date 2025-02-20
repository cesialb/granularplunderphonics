#include "GranularPlunderphonicsProcessor.h"
#include "GranularPlunderphonicsController.h"
#include "GranularPlunderphonicsIDs.h"
#include "version.h"
#include "public.sdk/source/main/pluginfactory.h"

#define stringPluginName "Granular Plunderphonics"

BEGIN_FACTORY_DEF(stringVendor, 
                  stringURL,
                  stringEmail)

    DEF_CLASS2(INLINE_UID_FROM_FUID(GranularPlunderphonicsProcessorUID),
              PClassInfo::kManyInstances,
              kVstAudioEffectClass,
              stringPluginName,
              Vst::kDistributable,
              Vst::PlugType::kFxGenerator,
              FULL_VERSION_STR,
              kVstVersionString,
              GranularPlunderphonics::GranularPlunderphonicsProcessor::createInstance)

END_FACTORY
