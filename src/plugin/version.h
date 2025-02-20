/**
* @file version.h
 * @brief Version information for the GranularPlunderphonics plugin
 */

#pragma once

// Version platform definitions (will use VST3 SDK when available)
#ifndef PLATFORM_64
    #if defined(_WIN64) || defined(__LP64__) || defined(__amd64__) || defined(__arm64__) || defined(__aarch64__)
        #define PLATFORM_64 1
    #else
        #define PLATFORM_64 0
    #endif
#endif

// Plain version numbers
#define GRANULAR_PLUNDERPHONICS_VERSION_MAJOR 0
#define GRANULAR_PLUNDERPHONICS_VERSION_MINOR 1
#define GRANULAR_PLUNDERPHONICS_VERSION_PATCH 0

// Version string (e.g., "0.1.0")
#define GRANULAR_PLUNDERPHONICS_VERSION_STR "0.1.0"

// Vendor specific version number (hexadecimal, format: 0xMMMMmmpp)
#define GRANULAR_PLUNDERPHONICS_VERSION_NUM 0x00000100

// VST3 SDK Version info placeholder (will be set by SDK when available)
#ifndef FULL_VERSION_STR
    #define FULL_VERSION_STR "0.1.0"
#endif
#define VST_SDK_VERSION "VST 3.7.x"

// Build timestamp (filled by build system)
#ifndef GRANULAR_PLUNDERPHONICS_BUILD_TIMESTAMP
    #define GRANULAR_PLUNDERPHONICS_BUILD_TIMESTAMP __DATE__ " " __TIME__
#endif