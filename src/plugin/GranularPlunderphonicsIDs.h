/**
 * @file GranularPlunderphonicsIDs.h
 * @brief Defines constants and IDs used throughout the GranularPlunderphonics plugin
 */

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace GranularPlunderphonics {

//------------------------------------------------------------------------
// Project-specific constants
//------------------------------------------------------------------------

// Plugin vendor and other info
static const Steinberg::FUID kGranularPlunderphonicsProcessorUID(0x12345678, 0x1234, 0x1234, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0);
static const Steinberg::FUID kGranularPlunderphonicsControllerUID(0x87654321, 0x4321, 0x4321, 0xF0, 0xED, 0xCB, 0xA9, 0x87, 0x65, 0x43, 0x21);

static const Steinberg::Vst::String128 kGranularPlunderphonicsName = u"Granular Plunderphonics";
static const char* kGranularPlunderphonicsVendor = "YourCompany";
static const char* kGranularPlunderphonicsURL = "www.example.com";
static const char* kGranularPlunderphonicsEmail = "your.email@example.com";

// Plugin category
static const Steinberg::Vst::PlugType kGranularPlunderphonicsCategory = Steinberg::Vst::PlugType::kFxGenerator;

// Plugin version (update with actual version)
static const int kGranularPlunderphonicsVersion = 0x00000100; // 0.1.0

//------------------------------------------------------------------------
// Parameter IDs
//------------------------------------------------------------------------
enum GranularParameters : Steinberg::Vst::ParamID {
    // No parameters yet - will be added as the plugin develops
    kBypassId = 1000,
};

//------------------------------------------------------------------------
// Error codes
//------------------------------------------------------------------------
enum ErrorCodes {
    kNoError = 0,
    kInitializationError = -1000,
    kProcessingError = -1001,
    kMemoryError = -1002,
    kInvalidParameter = -1003,
    // Add more error codes as needed
};

} // namespace GranularPlunderphonics