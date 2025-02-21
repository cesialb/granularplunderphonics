// GranularPlunderphonicsIDs.h
#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/base/fplatform.h"

namespace GranularPlunderphonics {

    // Unique Plugin IDs - replace these with your own unique IDs!
    static const Steinberg::FUID kGranularPlunderphonicsProcessorUID(
        0x12345678,
        0x12341234,
        0x12341234,
        0x12345678
    );

    static const Steinberg::FUID kGranularPlunderphonicsControllerUID(
        0x87654321,
        0x43214321,
        0x43214321,
        0x87654321
    );

    // Plugin Parameters
    enum GranularParameterIDs {
        kParamGrainSize = 0,
        kParamGrainShape,
        kParamGrainDensity,
        // Add more parameters as needed
    };

} // namespace GranularPlunderphonics
