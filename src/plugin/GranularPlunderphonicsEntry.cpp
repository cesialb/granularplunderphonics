/**
* @file GranularPlunderphonicsEntry.cpp
 * @brief Entry point for the GranularPlunderphonics VST3 plugin
 *
 * This is a placeholder that will be replaced with the actual VST3 factory
 * implementation when the SDK is available.
 */

#include "GranularPlunderphonicsProcessor.h"
#include "GranularPlunderphonicsController.h"
#include "GranularPlunderphonicsIDs.h"
#include "version.h"

#include <iostream>

// When VST3 SDK is not available, this provides a simple placeholder
namespace GranularPlunderphonics {

    // Placeholder for VST3 Plugin factory
    class PluginFactory {
    public:
        static void initialize() {
            std::cout << "GranularPlunderphonics plugin factory initialized" << std::endl;
            std::cout << "Plugin name: Granular Plunderphonics" << std::endl;
            std::cout << "Version: " << GRANULAR_PLUNDERPHONICS_VERSION_STR << std::endl;
            std::cout << "Vendor: " << kGranularPlunderphonicsVendor << std::endl;
            std::cout << "This is a placeholder until VST3 SDK is available" << std::endl;
        }
    };

    // When this file is compiled as part of a real build with VST3 SDK,
    // this code will be replaced with the actual factory implementation
    void initializePlugin() {
        PluginFactory::initialize();
    }

}  // namespace GranularPlunderphonics

// Simple entry point function that will be replaced by the VST3 SDK macros
extern "C" {
    void GranularPlunderphonicsEntry_Initialize() {
        GranularPlunderphonics::initializePlugin();
    }
}