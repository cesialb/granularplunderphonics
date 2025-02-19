/**
* @file version.h
 * @brief Version information for the GranularPlunderphonics plugin
 */

#pragma once

#include "pluginterfaces/base/fplatform.h"

// Plain version numbers
#define GRANULAR_PLUNDERPHONICS_VERSION_MAJOR 0
#define GRANULAR_PLUNDERPHONICS_VERSION_MINOR 1
#define GRANULAR_PLUNDERPHONICS_VERSION_PATCH 0

// Version string (e.g., "0.1.0")
#define GRANULAR_PLUNDERPHONICS_VERSION_STR "0.1.0"

// Vendor specific version number (hexadecimal, format: 0xMMMMmmpp)
#define GRANULAR_PLUNDERPHONICS_VERSION_NUM 0x00000100

// VST3 SDK Version used for building the plugin
#define VST_SDK_VERSION FULL_VERSION_STR

// Build timestamp (filled by build system)
#ifndef GRANULAR_PLUNDERPHONICS_BUILD_TIMESTAMP
#define GRANULAR_PLUNDERPHONICS_BUILD_TIMESTAMP __DATE__ " " __TIME__
#endif