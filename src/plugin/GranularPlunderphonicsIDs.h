/**
 * @file GranularPlunderphonicsIDs.h
 * @brief Defines constants and IDs used throughout the GranularPlunderphonics plugin
 */

#pragma once

#include <string>

// Forward declarations for VST3 types (will be replaced when SDK is available)
namespace Steinberg {
    namespace Vst {
        typedef unsigned int ParamID;
        enum PlugType { kFxGenerator = 1 };
        typedef wchar_t String128[128];
    }

    // FUID placeholder (will be replaced with actual implementation)
    class FUID {
    public:
        FUID(unsigned int l1, unsigned int l2, unsigned int l3,
             unsigned char b1, unsigned char b2, unsigned char b3, unsigned char b4,
             unsigned char b5, unsigned char b6, unsigned char b7, unsigned char b8) {}
    };
}

namespace GranularPlunderphonics {

//------------------------------------------------------------------------
// Project-specific constants
//------------------------------------------------------------------------

// Plugin vendor and other info
static const ::Steinberg::FUID kGranularPlunderphonicsProcessorUID(0x12345678, 0x1234, 0x1234, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0);
static const ::Steinberg::FUID kGranularPlunderphonicsControllerUID(0x87654321, 0x4321, 0x4321, 0xF0, 0xED, 0xCB, 0xA9, 0x87, 0x65, 0x43, 0x21);

// Define String128 literals in a way that works without VST3 SDK
static const ::Steinberg::Vst::String128 kGranularPlunderphonicsName = {0};  // Will be properly initialized with SDK
static const char* kGranularPlunderphonicsVendor = "YourCompany";
static const char* kGranularPlunderphonicsURL = "www.example.com";
static const char* kGranularPlunderphonicsEmail = "your.email@example.com";

// Plugin category
static const ::Steinberg::Vst::PlugType kGranularPlunderphonicsCategory = ::Steinberg::Vst::kFxGenerator;

// Plugin version (update with actual version)
static const int kGranularPlunderphonicsVersion = 0x00000100; // 0.1.0

//------------------------------------------------------------------------
// Parameter IDs
//------------------------------------------------------------------------
enum GranularParameters : ::Steinberg::Vst::ParamID {
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