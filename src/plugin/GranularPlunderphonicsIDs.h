#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/base/fplatform.h"

namespace GranularPlunderphonics {

    // Unique Plugin IDs - replace these with your own unique IDs!
    // Generated using https://www.guidgenerator.com/

    // Processor Class ID: {12345678-1234-1234-1234-123456789ABC}
    static const Steinberg::FUID kGranularPlunderphonicsProcessorUID(
        0x12345678, 0x1234, 0x1234, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0
    );

    // Controller Class ID: {87654321-4321-4321-4321-CBA987654321}
    static const Steinberg::FUID kGranularPlunderphonicsControllerUID(
        0x87654321, 0x4321, 0x4321, 0xF0, 0xED, 0xCB, 0xA9, 0x87, 0x65, 0x43, 0x21
    );

} // namespace GranularPlunderphonics