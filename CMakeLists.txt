cmake_minimum_required(VERSION 3.15)
project(GranularPlunderphonics VERSION 0.1.0)

# Add Homebrew paths explicitly
list(APPEND CMAKE_PREFIX_PATH
        "/opt/homebrew/Cellar/fftw/3.3.10_2"
        "/opt/homebrew/Cellar/libsamplerate/0.2.2"  # Add this line
)
set(FFTW_ROOT "/opt/homebrew/Cellar/fftw/3.3.10_2")
set(SAMPLERATE_ROOT "/opt/homebrew/Cellar/libsamplerate/0.2.2")  # Add this line

# C++17 Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Define DEVELOPMENT for Debug builds and RELEASE for Release builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEVELOPMENT=1 -D_DEBUG=1)
else()
    add_definitions(-DRELEASE=1 -DNDEBUG=1)
endif()

# Set VST3 SDK path and options
set(VST3_SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vst3sdk" CACHE PATH "Path to VST3 SDK")
set(SMTG_ADD_VST3_HOSTING_SAMPLES OFF CACHE BOOL "Don't build hosting samples")
set(SMTG_ADD_VST3_PLUGINS_SAMPLES OFF CACHE BOOL "Don't build plugin samples")
set(SMTG_CREATE_PLUGIN_LINK ON CACHE BOOL "Create symlink for plugins")

# Verify SDK exists
if(NOT EXISTS ${VST3_SDK_ROOT}/CMakeLists.txt)
    message(FATAL_ERROR "VST3 SDK not found. Please initialize submodules or specify VST3_SDK_ROOT.")
endif()

# Find dependencies
find_package(spdlog REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFTW REQUIRED fftw3f)
add_subdirectory("/Users/raydiator/Code/JUCE" "${CMAKE_BINARY_DIR}/JUCE_build")

# Manual setup for libsamplerate
set(SAMPLERATE_LIBRARIES "${SAMPLERATE_ROOT}/lib/libsamplerate.dylib")
set(SAMPLERATE_INCLUDE_DIRS "${SAMPLERATE_ROOT}/include")

# Add explicit link directories
link_directories(
        ${FFTW_ROOT}/lib
        ${SAMPLERATE_ROOT}/lib  # Add this line
)

# VST3 SDK setup - this must come before our own targets
add_subdirectory(${VST3_SDK_ROOT} vst3sdk)

# Make VST3 SDK headers available
include_directories(
        ${VST3_SDK_ROOT}
        ${VST3_SDK_ROOT}/pluginterfaces
        ${VST3_SDK_ROOT}/base
        ${VST3_SDK_ROOT}/public.sdk
        ${SAMPLERATE_INCLUDE_DIRS}  # Add this line
)

# Enable testing
enable_testing()

# Add subdirectories
add_subdirectory(src)
add_subdirectory(test)